#ifndef OIDC_UTILITIES_H
#define OIDC_UTILITIES_H

#include <stddef.h>

int strstarts(const char* str, const char* pre) ;
int isValid(const char* c) ;
void clearFree(void* p, size_t len);
void clearFreeString(char* s);
char* oidc_sprintf(const char* fmt, ...) ;
char* oidc_strcat(const char* str, const char* suf) ;
unsigned short getRandomPort() ;
char* portToUri(unsigned short port) ;

char* arrToListString(char** arr, size_t size, char delimiter, int surround) ;
int listStringToArray(const char* str, char delimiter, char** arr) ;
size_t strCountChar(const char* s, char c) ;
char* getDateString() ;
unsigned short getPortFromUri(const char* uri) ;
char* strelim(char* str, char c) ;
char* strelimIfFollowed(char str[], char c, char f) ;
void assertOidcDirExists() ;

#endif //OIDC_UTILITIES_H
