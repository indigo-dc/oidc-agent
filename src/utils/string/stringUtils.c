#define _GNU_SOURCE
#include "stringUtils.h"

#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <time.h>

#include "utils/logger.h"
#include "utils/memory.h"
#include "utils/oidc_error.h"

/** @fn int strValid(const char* c)
 * @brief checks if a string contains a valid value, meaning it is not empty,
 * NULL or "(null)"
 * @param c the string to be checked
 * @return 1 if the string is valid; 0 if not
 */
int strValid(const char* c) {
  return c && !strequal("", c) && !strequal("(null)", c) &&
         !strequal("null", c);
}

/** @fn strstarts(const char* str, const char* pre)
 * @brief checks if a string starts with a given string
 * @param str the string to be checked
 * @param pre the prefix \p str might start with
 * @return 1 if str starts with pre; 0 if not
 */
int strstarts(const char* str, const char* pre) {
  if (str == NULL || pre == NULL) {
    oidc_setArgNullFuncError(__func__);
    return 0;
  }
  return strncmp(pre, str, strlen(pre)) == 0;
}

int strEnds(const char* str, const char* suf) {
  if (str == NULL || suf == NULL) {
    oidc_setArgNullFuncError(__func__);
    return 0;
  }
  size_t lenstr    = strlen(str);
  size_t lensuffix = strlen(suf);
  if (lensuffix > lenstr) {
    return 0;
  }
  return strncmp(str + lenstr - lensuffix, suf, lensuffix) == 0;
}

int strEndsNot(const char* str, const char* suf) {
  return strEnds(str, suf) == 0 ? 1 : 0;
}

char* oidc_sprintf(const char* fmt, ...) {
  if (fmt == NULL) {
    oidc_setArgNullFuncError(__func__);
    return NULL;
  }
  va_list args;
  va_start(args, fmt);
  char* ret = oidc_vsprintf(fmt, args);
  va_end(args);
  return ret;
}

char* oidc_vsprintf(const char* fmt, va_list args) {
  if (fmt == NULL) {
    oidc_setArgNullFuncError(__func__);
    return NULL;
  }
  va_list orig;
  va_copy(orig, args);
  size_t len = vsnprintf(NULL, 0, fmt, args);
  char*  s   = secAlloc(sizeof(char) * (len + 1));
  if (s == NULL) {
    return NULL;
  }
  vsprintf(s, fmt, orig);
  va_end(orig);
  return s;
}

char* oidc_strcat(const char* str, const char* suf) {
  if (str == NULL || suf == NULL) {
    oidc_setArgNullFuncError(__func__);
    return NULL;
  }
  return oidc_sprintf("%s%s", str, suf);
}

char* oidc_strcopy(const char* str) {
  if (str == NULL) {
    oidc_setArgNullFuncError(__func__);
    return NULL;
  }
  return oidc_sprintf("%s", str);
}

char* oidc_strncopy(const char* str, int len) {
  if (str == NULL || len == 0) {
    oidc_setArgNullFuncError(__func__);
    return NULL;
  }
  int len_str = (int)strlen(str);
  return oidc_sprintf("%.*s", len_str < len ? len_str : len, str);
}

/** @fn char* getDateString()
 * @brief returns the current date in YYYY-mm-dd format
 * @returns a pointer to the formated date. Has to be freed after usage
 */
char* getDateString() {
  char* s = secAlloc(sizeof(char) * (10 + 1));
  if (s == NULL) {
    return NULL;
  }
  time_t now = time(NULL);
#ifdef __MINGW32__
  struct tm* t = localtime(&now);
  if (t == NULL) {
#else
  struct tm* t = secAlloc(sizeof(struct tm));
  if ((t = localtime_r(&now, t)) == NULL) {
#endif
    oidc_setErrnoError();
    secFree(t);
    return NULL;
  }
  strftime(s, 10 + 1, "%F", t);
  secFree(t);
  return s;
}

/**
 * eliminates a character c if it is followed by character f
 */
char* strelimIfFollowed(char* str, char c, char f) {
  if (str == NULL) {
    oidc_setArgNullFuncError(__func__);
    return str;
  }
  size_t len = strlen(str);
  size_t i, j;
  for (i = 0; i < len - 1; i++) {
    if (str[i] == c && str[i + 1] == f) {
      for (j = i; j < len - 1; j++) { str[j] = str[j + 1]; }
      str[j] = '\0';
    }
  }
  return str;
}

/**
 * eliminates a character c if it the previous character is f
 */
char* strelimIfAfter(char* str, char c, char f) {
  if (str == NULL) {
    oidc_setArgNullFuncError(__func__);
    return str;
  }
  size_t len = strlen(str);
  size_t i, j;
  for (i = 1; i < len - 1; i++) {
    if (str[i] == c && str[i - 1] == f) {
      for (j = i; j < len - 1; j++) { str[j] = str[j + 1]; }
      str[j] = '\0';
    }
  }
  return str;
}
char* strelim(char str[], char c) {
  if (str == NULL) {
    oidc_setArgNullFuncError(__func__);
    return NULL;
  }
  size_t len = strlen(str);
  size_t i, j;
  for (i = 0; i < len; i++) {
    if (str[i] == c) {
      for (j = i; j < len; j++) { str[j] = str[j + 1]; }
    }
  }
  return str;
}

char* strremove(char* str, const char* sub) {
  char *p, *q, *r;
  if ((q = r = strstr(str, sub)) != NULL) {
    size_t len = strlen(sub);
    while ((r = strstr(p = r + len, sub)) != NULL) {
      while (p < r) *q++ = *p++;
    }
    while ((*q++ = *p++) != '\0') continue;
  }
  return str;
}

size_t strCountChar(const char* s, char c) {
  if (s == NULL) {
    oidc_setArgNullFuncError(__func__);
    return 0;
  }
  int i;
  for (i = 0; s[i]; s[i] == c ? i++ : *s++)
    ;
  return i;
}

int strequal(const char* a, const char* b) {
  if (a == NULL && b == NULL) {
    return 1;
  }
  if (a == NULL || b == NULL) {
    return 0;
  }
  return strcmp(a, b) == 0 ? 1 : 0;
}

int strcaseequal(const char* a, const char* b) {
  if (a == NULL && b == NULL) {
    return 1;
  }
  if (a == NULL || b == NULL) {
    return 0;
  }
  return strcasecmp(a, b) == 0 ? 1 : 0;
}

char* escapeCharInStr(const char* str, char c) {
  if (str == NULL) {
    oidc_setArgNullFuncError(__func__);
    return NULL;
  }
  char*        s   = oidc_strcopy(str);
  const char*  pos = s;
  unsigned int rel = pos - s;
  while (rel < strlen(s) && (pos = strchr(s + rel, c)) != NULL) {
    rel       = pos - s;
    char* tmp = secRealloc(s, strlen(s) + 1 + 1);
    if (tmp == NULL) {
      secFree(s);
      return NULL;
    }
    s = tmp;
    memmove(s + rel + 1, s + rel, strlen(s + rel) + 1);
    s[rel] = '\\';
    rel += 2;
  }
  return s;
}

char* strlower(const char* str) {
  char* lower = oidc_strcopy(str);
  for (int i = 0; lower[i]; i++) { lower[i] = tolower(lower[i]); }
  return lower;
}

int strSubStringCase(const char* h, const char* n) {
  if (h == NULL || n == NULL) {
    return 0;
  }
#ifdef __MINGW32__
  char* h_l = strlower(h);
  char* n_l = strlower(h);
  int   ret = strstr(h, n) != NULL;
  secFree(h_l);
  secFree(n_l);
  return ret;
#else
  return strcasestr(h, n) != NULL;
#endif
}

int strSubString(const char* h, const char* n) {
  if (h == NULL || n == NULL) {
    return 0;
  }
  return strstr(h, n) != NULL;
}

char* withTrailingSlash(const char* str) {
  if (str[strlen(str) - 1] == '/') {
    return oidc_strcopy(str);
  }
  return oidc_strcat(str, "/");
}

size_t oidc_strlen(const char* str) {
  if (str == NULL) {
    return 0;
  }
  return strlen(str);
}

int strToInt(const char* str) {
  if (str == NULL) {
    oidc_setArgNullFuncError(__func__);
    return 0;
  }
  int i = 0;
  sscanf(str, "%d", &i);
  return i;
}

unsigned long strToULong(const char* str) {
  if (str == NULL) {
    oidc_setArgNullFuncError(__func__);
    return 0;
  }
  unsigned long l = 0;
  sscanf(str, "%lu", &l);
  return l;
}

unsigned char strToUChar(const char* str) {
  if (str == NULL) {
    oidc_setArgNullFuncError(__func__);
    return 0;
  }
  unsigned char c = 0;
  sscanf(str, "%hhu", &c);
  return c;
}

unsigned short strToUShort(const char* str) {
  if (str == NULL) {
    oidc_setArgNullFuncError(__func__);
    return 0;
  }
  unsigned short s = 0;
  sscanf(str, "%hu", &s);
  return s;
}

void debugPrintVaArg(const char* function, const char* fmt, va_list args) {
  va_list copy;
  va_copy(copy, args);
  printf("In function %s arguments are: ", function);
  vprintf(fmt, copy);
  va_end(copy);
  printf("\n");
}

char firstNonWhiteSpaceChar(const char* str) {
  for (size_t i = 0; i < strlen(str); i++) {
    if (!isspace(str[i])) {
      return str[i];
    }
  }
  return 0;
}

void strReplaceChar(char* str, char orig, char rep) {
  char* ix = str;
  int   n  = 0;
  while ((ix = strchr(ix, orig)) != NULL) {
    *ix++ = rep;
    n++;
  }
}

char* oidc_pathcat(const char* a, const char* b) {
  return lastChar(a) == '/' ? oidc_strcat(a, b) : oidc_sprintf("%s/%s", a, b);
}

char* repeatChar(char c, size_t n) {
  char* str = secAlloc(n + 1);
  memset(str, c, n);
  return str;
}

char* strreplace(const char* str, const char* old, const char* new) {
  if (str == NULL || old == NULL) {
    return NULL;
  }
  if (new == NULL) {
    new = "";
  }

  size_t old_len = strlen(old);
  char*  str_tmp = oidc_strcopy(str);
  char*  result  = oidc_strcopy("");
  char*  front   = str_tmp;
  char*  pos     = strstr(front, old);
  while (pos != NULL) {
    *pos      = '\0';
    char* tmp = oidc_sprintf("%s%s%s", result, front, new);
    secFree(result);
    result = tmp;
    front  = pos + old_len;
    pos    = strstr(front, old);
  }
  char* tmp = oidc_strcat(result, front);
  secFree(result);
  result = tmp;
  secFree(str_tmp);
  return result;
}

void strcutafterfirst(char* str, int c) {
  char* pos = strchr(str, c);
  if (pos != NULL) {
    *pos = '\0';
  }
}

void strcutafterlast(char* str, int c) {
  char* pos = strrchr(str, c);
  if (pos != NULL) {
    *pos = '\0';
  }
}
