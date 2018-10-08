#include "stringUtils.h"

#include "../oidc_error.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <time.h>

/** @fn int strValid(const char* c)
 * @brief checks if a string contains a valid value, meaning it is not empty,
 * NULL or "(null)"
 * @param c the string to be checked
 * @return 1 if the string is valid; 0 if not
 */
int strValid(const char* c) {
  return c && strcmp("", c) != 0 && strcmp("(null)", c) != 0 &&
         strcmp("null", c) != 0;
}

/** @fn strstarts(const char* str, const char* pre)
 * @brief checks if a string starts with a given string
 * @param str the string to be checked
 * @param pre the prefix \p str might start with
 * @return 1 if str starts with pre; 0 if not
 */
int strstarts(const char* str, const char* pre) {
  return strncmp(pre, str, strlen(pre)) == 0;
}

int strEnds(const char* str, const char* suf) {
  if (str == NULL || suf == NULL) {
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
  va_list args, orig;
  va_start(args, fmt);
  va_start(orig, fmt);
  char* s = calloc(sizeof(char), vsnprintf(NULL, 0, fmt, args) + 1);
  if (s == NULL) {
    oidc_errno = OIDC_EALLOC;
    return NULL;
  }
  vsprintf(s, fmt, orig);
  return s;
}

char* oidc_strcat(const char* str, const char* suf) {
  return oidc_sprintf("%s%s", str, suf);
}
char* oidc_strcopy(const char* str) { return oidc_sprintf("%s", str); }

/** @fn char* getDateString()
 * @brief returns the current date in YYYY-mm-dd format
 * @returns a pointer to the formated date. Has to be freed after usage
 */
char* getDateString() {
  char*      s   = calloc(sizeof(char), 10 + 1);
  time_t     now = time(NULL);
  struct tm* t   = localtime(&now);
  strftime(s, 10 + 1, "%F", t);
  return s;
}

/**
 * eliminates a character c if it is followed by character f
 */
char* strelimIfFollowed(char* str, char c, char f) {
  if (!strValid(str)) {
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
  // syslog(LOG_AUTHPRIV|LOG_DEBUG, "In strelim eliminating '%c'; new string is
  // '%s'", c, str);
  return str;
}

char* strelim(char str[], char c) {
  if (str == NULL) {
    return NULL;
  }
  size_t len = strlen(str);
  size_t i, j;
  for (i = 0; i < len; i++) {
    if (str[i] == c) {
      for (j = i; j < len; j++) { str[j] = str[j + 1]; }
    }
  }
  // syslog(LOG_AUTHPRIV|LOG_DEBUG, "In strelim eliminating '%c'; new string is
  // '%s'", c, str);
  return str;
}

size_t strCountChar(const char* s, char c) {
  int i;
  for (i = 0; s[i]; s[i] == c ? i++ : *s++)
    ;
  return i;
}

int strequal(const char* a, const char* b) { return strcmp(a, b) == 0 ? 1 : 0; }

int strcaseequal(const char* a, const char* b) {
  return strcasecmp(a, b) == 0 ? 1 : 0;
}

char* escapeCharInStr(const char* str, char c) {
  if (str == NULL) {
    return NULL;
  }
  char*        s   = oidc_strcopy(str);
  const char*  pos = s;
  unsigned int rel = pos - s;
  while (rel < strlen(s) && (pos = strchr(s + rel, c)) != NULL) {
    rel = pos - s;
    s   = realloc(s, strlen(s) + 1 + 1);
    memmove(s + rel + 1, s + rel, strlen(s + rel) + 1);
    s[rel] = '\\';
    rel += 2;
  }
  return s;
}
