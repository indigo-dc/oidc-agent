/*
 Author: José Bollo <jobol@nonadev.net>

 https://gitlab.com/jobol/mustach

 SPDX-License-Identifier: ISC
*/

#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#ifdef _WIN32
#include <malloc.h>
#endif

#include "mustach.h"

struct iwrap {
	int (*emit)(void *closure, const char *buffer, size_t size, int escape, FILE *file);
	void *closure; /* closure for: enter, next, leave, emit, get */
	int (*put)(void *closure, const char *name, int escape, FILE *file);
	void *closure_put; /* closure for put */
	int (*enter)(void *closure, const char *name);
	int (*next)(void *closure);
	int (*leave)(void *closure);
	int (*get)(void *closure, const char *name, struct mustach_sbuf *sbuf);
	int (*partial)(void *closure, const char *name, struct mustach_sbuf *sbuf);
	void *closure_partial; /* closure for partial */
	int flags;
};

struct prefix {
	size_t len;
	const char *start;
	struct prefix *prefix;
};

#if !defined(NO_OPEN_MEMSTREAM)
static FILE *memfile_open(char **buffer, size_t *size)
{
	return open_memstream(buffer, size);
}
static void memfile_abort(FILE *file, char **buffer, size_t *size)
{
	fclose(file);
	free(*buffer);
	*buffer = NULL;
	*size = 0;
}
static int memfile_close(FILE *file, char **buffer, size_t *size)
{
	int rc;

	/* adds terminating null */
	rc = fputc(0, file) ? MUSTACH_ERROR_SYSTEM : 0;
	fclose(file);
	if (rc == 0)
		/* removes terminating null of the length */
		(*size)--;
	else {
		free(*buffer);
		*buffer = NULL;
		*size = 0;
	}
	return rc;
}
#else
static FILE *memfile_open(char **buffer, size_t *size)
{
	/*
	 * We can't provide *buffer and *size as open_memstream does but
	 * at least clear them so the caller won't get bad data.
	 */
	*buffer = NULL;
	*size = 0;

	return tmpfile();
}
static void memfile_abort(FILE *file, char **buffer, size_t *size)
{
	fclose(file);
	*buffer = NULL;
	*size = 0;
}
static int memfile_close(FILE *file, char **buffer, size_t *size)
{
	int rc;
	size_t s;
	char *b;

	s = (size_t)ftell(file);
	b = malloc(s + 1);
	if (b == NULL) {
		rc = MUSTACH_ERROR_SYSTEM;
		errno = ENOMEM;
		s = 0;
	} else {
		rewind(file);
		if (1 == fread(b, s, 1, file)) {
			rc = 0;
			b[s] = 0;
		} else {
			rc = MUSTACH_ERROR_SYSTEM;
			free(b);
			b = NULL;
			s = 0;
		}
	}
	*buffer = b;
	*size = s;
	return rc;
}
#endif

static inline void sbuf_reset(struct mustach_sbuf *sbuf)
{
	sbuf->value = NULL;
	sbuf->freecb = NULL;
	sbuf->closure = NULL;
	sbuf->length = 0;
}

static inline void sbuf_release(struct mustach_sbuf *sbuf)
{
	if (sbuf->releasecb)
		sbuf->releasecb(sbuf->value, sbuf->closure);
}

static inline size_t sbuf_length(struct mustach_sbuf *sbuf)
{
	size_t length = sbuf->length;
	if (length == 0 && sbuf->value != NULL)
		length = strlen(sbuf->value);
	return length;
}

static int iwrap_emit(void *closure, const char *buffer, size_t size, int escape, FILE *file)
{
	size_t i, j, r;

	(void)closure; /* unused */

	if (!escape)
		return fwrite(buffer, 1, size, file) != size ? MUSTACH_ERROR_SYSTEM : MUSTACH_OK;

	r = i = 0;
	while (i < size) {
		j = i;
		while (j < size && buffer[j] != '<' && buffer[j] != '>' && buffer[j] != '&' && buffer[j] != '"')
			j++;
		if (j != i && fwrite(&buffer[i], j - i, 1, file) != 1)
			return MUSTACH_ERROR_SYSTEM;
		if (j < size) {
			switch(buffer[j++]) {
			case '<':
				r = fwrite("&lt;", 4, 1, file);
				break;
			case '>':
				r = fwrite("&gt;", 4, 1, file);
				break;
			case '&':
				r = fwrite("&amp;", 5, 1, file);
				break;
			case '"':
				r = fwrite("&quot;", 6, 1, file);
				break;
			}
			if (r != 1)
				return MUSTACH_ERROR_SYSTEM;
		}
		i = j;
	}
	return MUSTACH_OK;
}

static int iwrap_put(void *closure, const char *name, int escape, FILE *file)
{
	struct iwrap *iwrap = closure;
	int rc;
	struct mustach_sbuf sbuf;
	size_t length;

	sbuf_reset(&sbuf);
	rc = iwrap->get(iwrap->closure, name, &sbuf);
	if (rc >= 0) {
		length = sbuf_length(&sbuf);
		if (length)
			rc = iwrap->emit(iwrap->closure, sbuf.value, length, escape, file);
		sbuf_release(&sbuf);
	}
	return rc;
}

static int iwrap_partial(void *closure, const char *name, struct mustach_sbuf *sbuf)
{
	struct iwrap *iwrap = closure;
	int rc;
	FILE *file;
	size_t size;
	char *result;

	result = NULL;
	file = memfile_open(&result, &size);
	if (file == NULL)
		rc = MUSTACH_ERROR_SYSTEM;
	else {
		rc = iwrap->put(iwrap->closure_put, name, 0, file);
		if (rc < 0)
			memfile_abort(file, &result, &size);
		else {
			rc = memfile_close(file, &result, &size);
			if (rc == 0) {
				sbuf->value = result;
				sbuf->freecb = free;
				sbuf->length = size;
			}
		}
	}
	return rc;
}

static int emitprefix(struct iwrap *iwrap, FILE *file, struct prefix *prefix)
{
	if (prefix->prefix) {
		int rc = emitprefix(iwrap, file, prefix->prefix);
		if (rc < 0)
			return rc;
	}
	return prefix->len ? iwrap->emit(iwrap->closure, prefix->start, prefix->len, 0, file) : 0;
}

static int process(const char *template, size_t length, struct iwrap *iwrap, FILE *file, struct prefix *prefix)
{
	struct mustach_sbuf sbuf;
	char opstr[MUSTACH_MAX_DELIM_LENGTH], clstr[MUSTACH_MAX_DELIM_LENGTH];
	char name[MUSTACH_MAX_LENGTH + 1], c;
	const char *beg, *term, *end;
	struct { const char *name, *again; size_t length; unsigned enabled: 1, entered: 1; } stack[MUSTACH_MAX_DEPTH];
	size_t oplen, cllen, len, l;
	int depth, rc, enabled, stdalone;
	struct prefix pref;

	pref.prefix = prefix;
	end = template + (length ? length : strlen(template));
	opstr[0] = opstr[1] = '{';
	clstr[0] = clstr[1] = '}';
	oplen = cllen = 2;
	stdalone = enabled = 1;
	depth = pref.len = 0;
	for (;;) {
		/* search next openning delimiter */
		for (beg = template ; ; beg++) {
			c = beg == end ? '\n' : *beg;
			if (c == '\n') {
				l = (beg != end) + (size_t)(beg - template);
				if (stdalone != 2 && enabled) {
					if (beg != template /* don't prefix empty lines */) {
						rc = emitprefix(iwrap, file, &pref);
						if (rc < 0)
							return rc;
					}
					rc = iwrap->emit(iwrap->closure, template, l, 0, file);
					if (rc < 0)
						return rc;
				}
				if (beg == end) /* no more mustach */
					return depth ? MUSTACH_ERROR_UNEXPECTED_END : MUSTACH_OK;
				template += l;
				stdalone = 1;
				pref.len = 0;
			}
			else if (!isspace(c)) {
				if (stdalone == 2 && enabled) {
					rc = emitprefix(iwrap, file, &pref);
					if (rc < 0)
						return rc;
					pref.len = 0;
					stdalone = 0;
				}
				if (c == *opstr && end - beg >= (ssize_t)oplen) {
					for (l = 1 ; l < oplen && beg[l] == opstr[l] ; l++);
					if (l == oplen)
						break;
				}
				stdalone = 0;
			}
		}

		pref.start = template;
		pref.len = enabled ? (size_t)(beg - template) : 0;
		beg += oplen;

		/* search next closing delimiter */
		for (term = beg ; ; term++) {
			if (term == end)
				return MUSTACH_ERROR_UNEXPECTED_END;
			if (*term == *clstr && end - term >= (ssize_t)cllen) {
				for (l = 1 ; l < cllen && term[l] == clstr[l] ; l++);
				if (l == cllen)
					break;
			}
		}
		template = term + cllen;
		len = (size_t)(term - beg);
		c = *beg;
		switch(c) {
		case ':':
			stdalone = 0;
			if (iwrap->flags & Mustach_With_Colon)
				goto exclude_first;
			goto get_name;
		case '!':
		case '=':
			break;
		case '{':
			for (l = 0 ; l < cllen && clstr[l] == '}' ; l++);
			if (l < cllen) {
				if (!len || beg[len-1] != '}')
					return MUSTACH_ERROR_BAD_UNESCAPE_TAG;
				len--;
			} else {
				if (term[l] != '}')
					return MUSTACH_ERROR_BAD_UNESCAPE_TAG;
				template++;
			}
			c = '&';
			/*@fallthrough@*/
		case '&':
			stdalone = 0;
			/*@fallthrough@*/
		case '^':
		case '#':
		case '/':
		case '>':
exclude_first:
			beg++;
			len--;
			goto get_name;
		default:
			stdalone = 0;
get_name:
			while (len && isspace(beg[0])) { beg++; len--; }
			while (len && isspace(beg[len-1])) len--;
			if (len == 0 && !(iwrap->flags & Mustach_With_EmptyTag))
				return MUSTACH_ERROR_EMPTY_TAG;
			if (len > MUSTACH_MAX_LENGTH)
				return MUSTACH_ERROR_TAG_TOO_LONG;
			memcpy(name, beg, len);
			name[len] = 0;
			break;
		}
		if (stdalone)
			stdalone = 2;
		else if (enabled) {
			rc = emitprefix(iwrap, file, &pref);
			if (rc < 0)
				return rc;
			pref.len = 0;
		}
		switch(c) {
		case '!':
			/* comment */
			/* nothing to do */
			break;
		case '=':
			/* defines delimiters */
			if (len < 5 || beg[len - 1] != '=')
				return MUSTACH_ERROR_BAD_SEPARATORS;
			beg++;
			len -= 2;
			while (len && isspace(*beg))
				beg++, len--;
			while (len && isspace(beg[len - 1]))
				len--;
			for (l = 0; l < len && !isspace(beg[l]) ; l++);
			if (l == len || l > MUSTACH_MAX_DELIM_LENGTH)
				return MUSTACH_ERROR_BAD_SEPARATORS;
			oplen = l;
			memcpy(opstr, beg, l);
			while (l < len && isspace(beg[l])) l++;
			if (l == len || len - l > MUSTACH_MAX_DELIM_LENGTH)
				return MUSTACH_ERROR_BAD_SEPARATORS;
			cllen = len - l;
			memcpy(clstr, beg + l, cllen);
			break;
		case '^':
		case '#':
			/* begin section */
			if (depth == MUSTACH_MAX_DEPTH)
				return MUSTACH_ERROR_TOO_DEEP;
			rc = enabled;
			if (rc) {
				rc = iwrap->enter(iwrap->closure, name);
				if (rc < 0)
					return rc;
			}
			stack[depth].name = beg;
			stack[depth].again = template;
			stack[depth].length = len;
			stack[depth].enabled = enabled != 0;
			stack[depth].entered = rc != 0;
			if ((c == '#') == (rc == 0))
				enabled = 0;
			depth++;
			break;
		case '/':
			/* end section */
			if (depth-- == 0 || len != stack[depth].length || memcmp(stack[depth].name, name, len))
				return MUSTACH_ERROR_CLOSING;
			rc = enabled && stack[depth].entered ? iwrap->next(iwrap->closure) : 0;
			if (rc < 0)
				return rc;
			if (rc) {
				template = stack[depth++].again;
			} else {
				enabled = stack[depth].enabled;
				if (enabled && stack[depth].entered)
					iwrap->leave(iwrap->closure);
			}
			break;
		case '>':
			/* partials */
			if (enabled) {
				sbuf_reset(&sbuf);
				rc = iwrap->partial(iwrap->closure_partial, name, &sbuf);
				if (rc >= 0) {
					rc = process(sbuf.value, sbuf_length(&sbuf), iwrap, file, &pref);
					sbuf_release(&sbuf);
				}
				if (rc < 0)
					return rc;
			}
			break;
		default:
			/* replacement */
			if (enabled) {
				rc = iwrap->put(iwrap->closure_put, name, c != '&', file);
				if (rc < 0)
					return rc;
			}
			break;
		}
	}
}

int mustach_file(const char *template, size_t length, const struct mustach_itf *itf, void *closure, int flags, FILE *file)
{
	int rc;
	struct iwrap iwrap;

	/* check validity */
	if (!itf->enter || !itf->next || !itf->leave || (!itf->put && !itf->get))
		return MUSTACH_ERROR_INVALID_ITF;

	/* init wrap structure */
	iwrap.closure = closure;
	if (itf->put) {
		iwrap.put = itf->put;
		iwrap.closure_put = closure;
	} else {
		iwrap.put = iwrap_put;
		iwrap.closure_put = &iwrap;
	}
	if (itf->partial) {
		iwrap.partial = itf->partial;
		iwrap.closure_partial = closure;
	} else if (itf->get) {
		iwrap.partial = itf->get;
		iwrap.closure_partial = closure;
	} else {
		iwrap.partial = iwrap_partial;
		iwrap.closure_partial = &iwrap;
	}
	iwrap.emit = itf->emit ? itf->emit : iwrap_emit;
	iwrap.enter = itf->enter;
	iwrap.next = itf->next;
	iwrap.leave = itf->leave;
	iwrap.get = itf->get;
	iwrap.flags = flags;

	/* process */
	rc = itf->start ? itf->start(closure) : 0;
	if (rc == 0)
		rc = process(template, length, &iwrap, file, 0);
	if (itf->stop)
		itf->stop(closure, rc);
	return rc;
}

int mustach_fd(const char *template, size_t length, const struct mustach_itf *itf, void *closure, int flags, int fd)
{
	int rc;
	FILE *file;

	file = fdopen(fd, "w");
	if (file == NULL) {
		rc = MUSTACH_ERROR_SYSTEM;
		errno = ENOMEM;
	} else {
		rc = mustach_file(template, length, itf, closure, flags, file);
		fclose(file);
	}
	return rc;
}

int mustach_mem(const char *template, size_t length, const struct mustach_itf *itf, void *closure, int flags, char **result, size_t *size)
{
	int rc;
	FILE *file;
	size_t s;

	*result = NULL;
	if (size == NULL)
		size = &s;
	file = memfile_open(result, size);
	if (file == NULL)
		rc = MUSTACH_ERROR_SYSTEM;
	else {
		rc = mustach_file(template, length, itf, closure, flags, file);
		if (rc < 0)
			memfile_abort(file, result, size);
		else
			rc = memfile_close(file, result, size);
	}
	return rc;
}

int fmustach(const char *template, const struct mustach_itf *itf, void *closure, FILE *file)
{
	return mustach_file(template, 0, itf, closure, Mustach_With_AllExtensions, file);
}

int fdmustach(const char *template, const struct mustach_itf *itf, void *closure, int fd)
{
	return mustach_fd(template, 0, itf, closure, Mustach_With_AllExtensions, fd);
}

int mustach(const char *template, const struct mustach_itf *itf, void *closure, char **result, size_t *size)
{
	return mustach_mem(template, 0, itf, closure, Mustach_With_AllExtensions, result, size);
}

