#ifndef STRING_UTILS_H
#define STRING_UTILS_H

#include <stddef.h>

int strstarts(const char* str, const char* pre) ;
int strEnds(const char* str, const char* suf) ;
int strEndsNot(const char* str, const char* suf) ;
int strValid(const char* c) ;
size_t strCountChar(const char* s, char c) ;
char* strelim(char* str, char c) ;
int strequal(const char* a, const char* b) ;
int strcaseequal(const char* a, const char* b) ;
char* escapeCharInStr(const char* str, char c) ;

char* strelimIfFollowed(char str[], char c, char f) ;
char* oidc_sprintf(const char* fmt, ...) ;
char* oidc_strcat(const char* str, const char* suf) ;
char* oidc_strcopy(const char* str) ;

char* getDateString() ;

#endif // STRING_UTILS_H
