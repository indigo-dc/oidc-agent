#ifndef STRING_UTILS_H
#define STRING_UTILS_H

#include <stdarg.h>
#include <stddef.h>

int    strstarts(const char* str, const char* pre);
int    strEnds(const char* str, const char* suf);
int    strEndsNot(const char* str, const char* suf);
int    strValid(const char* c);
size_t strCountChar(const char* s, char c);
char*  strelim(char* str, char c);
char*  strremove(char* str, const char* sub);
int    strequal(const char* a, const char* b);
int    strcaseequal(const char* a, const char* b);
char*  escapeCharInStr(const char* str, char c);
int    strSubStringCase(const char* h, const char* n);
int    strSubString(const char* h, const char* n);
size_t oidc_strlen(const char* str);

char* strelimIfFollowed(char str[], char c, char f);
char* strelimIfAfter(char* str, char c, char f);
char* oidc_sprintf(const char* fmt, ...);
char* oidc_vsprintf(const char* fmt, va_list args);
char* oidc_strcat(const char* str, const char* suf);
char* oidc_strcopy(const char* str);
char* oidc_strncopy(const char* str, int len);
char* withTrailingSlash(const char* str);
char  firstNonWhiteSpaceChar(const char* str);
void strReplaceChar(char *str, char orig, char rep);

char*          getDateString();
unsigned long  strToULong(const char* str);
int            strToInt(const char* str);
unsigned char  strToUChar(const char* str);
unsigned short strToUShort(const char* str);

void debugPrintVaArg(const char* function, const char* fmt, va_list args);

#endif  // STRING_UTILS_H
