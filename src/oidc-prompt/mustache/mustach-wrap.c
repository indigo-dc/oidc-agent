/*
 Author: José Bollo <jobol@nonadev.net>

 https://gitlab.com/jobol/mustach

 SPDX-License-Identifier: ISC
*/

#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#ifdef _WIN32
#include <malloc.h>
#endif

#include "mustach.h"
#include "mustach-wrap.h"

#if !defined(INCLUDE_PARTIAL_EXTENSION)
# define INCLUDE_PARTIAL_EXTENSION ".mustache"
#endif

/* global hook for partials */
int (*mustach_wrap_get_partial)(const char *name, struct mustach_sbuf *sbuf) = NULL;

/* internal structure for wrapping */
struct wrap {
	/* original interface */
	const struct mustach_wrap_itf *itf;

	/* original closure */
	void *closure;

	/* flags */
	int flags;

	/* emiter callback */
	mustach_emit_cb_t *emitcb;

	/* write callback */
	mustach_write_cb_t *writecb;
};

/* length given by masking with 3 */
enum comp {
	C_no = 0,
	C_eq = 1,
	C_lt = 5,
	C_le = 6,
	C_gt = 9,
	C_ge = 10
};

enum sel {
	S_none = 0,
	S_ok = 1,
	S_objiter = 2,
	S_ok_or_objiter = S_ok | S_objiter
};

static enum comp getcomp(char *head, int sflags)
{
	return (head[0] == '=' && (sflags & Mustach_With_Equal)) ? C_eq
		: (head[0] == '<' && (sflags & Mustach_With_Compare)) ? (head[1] == '=' ? C_le : C_lt)
		: (head[0] == '>' && (sflags & Mustach_With_Compare)) ? (head[1] == '=' ? C_ge : C_gt)
		: C_no;
}

static char *keyval(char *head, int sflags, enum comp *comp)
{
	char *w, car, escaped;
	enum comp k;

	k = C_no;
	w = head;
	car = *head;
	escaped = (sflags & Mustach_With_EscFirstCmp) && (getcomp(head, sflags) != C_no);
	while (car && (escaped || (k = getcomp(head, sflags)) == C_no)) {
		if (escaped)
			escaped = 0;
		else
			escaped = ((sflags & Mustach_With_JsonPointer) ? car == '~' : car == '\\')
			    && (getcomp(head + 1, sflags) != C_no);
		if (!escaped)
			*w++ = car;
		head++;
		car = *head;
	}
	*w = 0;
	*comp = k;
	return k == C_no ? NULL : &head[k & 3];
}

static char *getkey(char **head, int sflags)
{
	char *result, *iter, *write, car;

	car = *(iter = *head);
	if (!car)
		result = NULL;
	else {
		result = write = iter;
		if (sflags & Mustach_With_JsonPointer)
		{
			while (car && car != '/') {
				if (car == '~')
					switch (iter[1]) {
					case '1': car = '/'; /*@fallthrough@*/
					case '0': iter++;
					}
				*write++ = car;
				car = *++iter;
			}
			*write = 0;
			while (car == '/')
				car = *++iter;
		}
		else
		{
			while (car && car != '.') {
				if (car == '\\' && (iter[1] == '.' || iter[1] == '\\'))
					car = *++iter;
				*write++ = car;
				car = *++iter;
			}
			*write = 0;
			while (car == '.')
				car = *++iter;
		}
		*head = iter;
	}
	return result;
}

static enum sel sel(struct wrap *w, const char *name)
{
	enum sel result;
	int i, j, sflags, scmp;
	char *key, *value;
	enum comp k;

	/* make a local writeable copy */
	size_t lenname = 1 + strlen(name);
	char buffer[lenname];
	char *copy = buffer;
	memcpy(copy, name, lenname);

	/* check if matches json pointer selection */
	sflags = w->flags;
	if (sflags & Mustach_With_JsonPointer) {
		if (copy[0] == '/')
			copy++;
		else
			sflags ^= Mustach_With_JsonPointer;
	}

	/* extract the value, translate the key and get the comparator */
	if (sflags & (Mustach_With_Equal | Mustach_With_Compare))
		value = keyval(copy, sflags, &k);
	else {
		k = C_no;
		value = NULL;
	}

	/* case of . alone if Mustach_With_SingleDot? */
	if (copy[0] == '.' && copy[1] == 0 /*&& (sflags & Mustach_With_SingleDot)*/)
		/* yes, select current */
		result = w->itf->sel(w->closure, NULL) ? S_ok : S_none;
	else
	{
		/* not the single dot, extract the first key */
		key = getkey(&copy, sflags);
		if (key == NULL)
			return 0;

		/* select the root item */
		if (w->itf->sel(w->closure, key))
			result = S_ok;
		else if (key[0] == '*'
		      && !key[1]
		      && !value
		      && !*copy
		      && (w->flags & Mustach_With_ObjectIter)
		      && w->itf->sel(w->closure, NULL))
			result = S_ok_or_objiter;
		else
			result = S_none;
		if (result == S_ok) {
			/* iterate the selection of sub items */
			key = getkey(&copy, sflags);
			while(result == S_ok && key) {
				if (w->itf->subsel(w->closure, key))
					/* nothing */;
				else if (key[0] == '*'
				      && !key[1]
				      && !value
				      && !*copy
				      && (w->flags & Mustach_With_ObjectIter))
					result = S_objiter;
				else
					result = S_none;
				key = getkey(&copy, sflags);
			}
		}
	}
	/* should it be compared? */
	if (result == S_ok && value) {
		if (!w->itf->compare)
			result = S_none;
		else {
			i = value[0] == '!';
			scmp = w->itf->compare(w->closure, &value[i]);
			switch (k) {
			case C_eq: j = scmp == 0; break;
			case C_lt: j = scmp < 0; break;
			case C_le: j = scmp <= 0; break;
			case C_gt: j = scmp > 0; break;
			case C_ge: j = scmp >= 0; break;
			default: j = i; break;
			}
			if (i == j)
				result = S_none;
		}
	}
	return result;
}

static int start(void *closure)
{
	struct wrap *w = closure;
	return w->itf->start ? w->itf->start(w->closure) : MUSTACH_OK;
}

static void stop(void *closure, int status)
{
	struct wrap *w = closure;
	if (w->itf->stop)
		w->itf->stop(w->closure, status);
}

static int write(struct wrap *w, const char *buffer, size_t size, FILE *file)
{
	int r;

	if (w->writecb)
		r = w->writecb(file, buffer, size);
	else
		r = fwrite(buffer, 1, size, file) == size ? MUSTACH_OK : MUSTACH_ERROR_SYSTEM;
	return r;
}

static int emit(void *closure, const char *buffer, size_t size, int escape, FILE *file)
{
	struct wrap *w = closure;
	int r;
	size_t s, i;
	char car;

	if (w->emitcb)
		r = w->emitcb(file, buffer, size, escape);
	else if (!escape)
		r = write(w, buffer, size, file);
	else {
		i = 0;
		r = MUSTACH_OK;
		while(i < size && r == MUSTACH_OK) {
			s = i;
			while (i < size && (car = buffer[i]) != '<' && car != '>' && car != '&' && car != '"')
				i++;
			if (i != s)
				r = write(w, &buffer[s], i - s, file);
			if (i < size && r == MUSTACH_OK) {
				switch(car) {
				case '<': r = write(w, "&lt;", 4, file); break;
				case '>': r = write(w, "&gt;", 4, file); break;
				case '&': r = write(w, "&amp;", 5, file); break;
				case '"': r = write(w, "&quot;", 6, file); break;
				}
				i++;
			}
		}
	}
	return r;
}

static int enter(void *closure, const char *name)
{
	struct wrap *w = closure;
	enum sel s = sel(w, name);
	return s == S_none ? 0 : w->itf->enter(w->closure, s & S_objiter);
}

static int next(void *closure)
{
	struct wrap *w = closure;
	return w->itf->next(w->closure);
}

static int leave(void *closure)
{
	struct wrap *w = closure;
	return w->itf->leave(w->closure);
}

static int getoptional(struct wrap *w, const char *name, struct mustach_sbuf *sbuf)
{
	enum sel s = sel(w, name);
	if (!(s & S_ok))
		return 0;
	return w->itf->get(w->closure, sbuf, s & S_objiter);
}

static int get(void *closure, const char *name, struct mustach_sbuf *sbuf)
{
	struct wrap *w = closure;
	if (getoptional(w, name, sbuf) <= 0) {
		if (w->flags & Mustach_With_ErrorUndefined)
			return MUSTACH_ERROR_UNDEFINED_TAG;
		sbuf->value = "";
	}
	return MUSTACH_OK;
}

static int get_partial_from_file(const char *name, struct mustach_sbuf *sbuf)
{
	static char extension[] = INCLUDE_PARTIAL_EXTENSION;
	size_t s;
	long pos;
	FILE *file;
	char *path, *buffer;

	/* allocate path */
	s = strlen(name);
	path = malloc(s + sizeof extension);
	if (path == NULL)
		return MUSTACH_ERROR_SYSTEM;

	/* try without extension first */
	memcpy(path, name, s + 1);
	file = fopen(path, "r");
	if (file == NULL) {
		memcpy(&path[s], extension, sizeof extension);
		file = fopen(path, "r");
	}
	free(path);

	/* if file opened */
	if (file == NULL)
		return MUSTACH_ERROR_PARTIAL_NOT_FOUND;

	/* compute file size */
	if (fseek(file, 0, SEEK_END) >= 0
	 && (pos = ftell(file)) >= 0
	 && fseek(file, 0, SEEK_SET) >= 0) {
		/* allocate value */
		s = (size_t)pos;
		buffer = malloc(s + 1);
		if (buffer != NULL) {
			/* read value */
			if (1 == fread(buffer, s, 1, file)) {
				/* force zero at end */
				sbuf->value = buffer;
				buffer[s] = 0;
				sbuf->freecb = free;
				fclose(file);
				return MUSTACH_OK;
			}
			free(buffer);
		}
	}
	fclose(file);
	return MUSTACH_ERROR_SYSTEM;
}

static int partial(void *closure, const char *name, struct mustach_sbuf *sbuf)
{
	struct wrap *w = closure;
	int rc;
	if (mustach_wrap_get_partial != NULL)
		rc = mustach_wrap_get_partial(name, sbuf);
	else if (w->flags & Mustach_With_PartialDataFirst) {
		if (getoptional(w, name, sbuf) > 0)
			rc = MUSTACH_OK;
		else
			rc = get_partial_from_file(name, sbuf);
	}
	else {
		rc = get_partial_from_file(name, sbuf);
		if (rc != MUSTACH_OK &&  getoptional(w, name, sbuf) > 0)
			rc = MUSTACH_OK;
	}
	if (rc != MUSTACH_OK)
		sbuf->value = "";
	return MUSTACH_OK;
}

const struct mustach_itf mustach_wrap_itf = {
	.start = start,
	.put = NULL,
	.enter = enter,
	.next = next,
	.leave = leave,
	.partial = partial,
	.get = get,
	.emit = emit,
	.stop = stop
};

static void wrap_init(struct wrap *wrap, const struct mustach_wrap_itf *itf, void *closure, int flags, mustach_emit_cb_t *emitcb, mustach_write_cb_t *writecb)
{
	if (flags & Mustach_With_Compare)
		flags |= Mustach_With_Equal;
	wrap->closure = closure;
	wrap->itf = itf;
	wrap->flags = flags;
	wrap->emitcb = emitcb;
	wrap->writecb = writecb;
}

int mustach_wrap_file(const char *template, size_t length, const struct mustach_wrap_itf *itf, void *closure, int flags, FILE *file)
{
	struct wrap w;
	wrap_init(&w, itf, closure, flags, NULL, NULL);
	return mustach_file(template, length, &mustach_wrap_itf, &w, flags, file);
}

int mustach_wrap_fd(const char *template, size_t length, const struct mustach_wrap_itf *itf, void *closure, int flags, int fd)
{
	struct wrap w;
	wrap_init(&w, itf, closure, flags, NULL, NULL);
	return mustach_fd(template, length, &mustach_wrap_itf, &w, flags, fd);
}

int mustach_wrap_mem(const char *template, size_t length, const struct mustach_wrap_itf *itf, void *closure, int flags, char **result, size_t *size)
{
	struct wrap w;
	wrap_init(&w, itf, closure, flags, NULL, NULL);
	return mustach_mem(template, length, &mustach_wrap_itf, &w, flags, result, size);
}

int mustach_wrap_write(const char *template, size_t length, const struct mustach_wrap_itf *itf, void *closure, int flags, mustach_write_cb_t *writecb, void *writeclosure)
{
	struct wrap w;
	wrap_init(&w, itf, closure, flags, NULL, writecb);
	return mustach_file(template, length, &mustach_wrap_itf, &w, flags, writeclosure);
}

int mustach_wrap_emit(const char *template, size_t length, const struct mustach_wrap_itf *itf, void *closure, int flags, mustach_emit_cb_t *emitcb, void *emitclosure)
{
	struct wrap w;
	wrap_init(&w, itf, closure, flags, emitcb, NULL);
	return mustach_file(template, length, &mustach_wrap_itf, &w, flags, emitclosure);
}

