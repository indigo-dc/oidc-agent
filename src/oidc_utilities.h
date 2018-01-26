#ifndef OIDC_UTILITIES_H
#define OIDC_UTILITIES_H

#include "../lib/list/src/list.h"

#include <stddef.h>

#define pass do {} while(0)

// Colors
#define C_RED   "\x1B[31m"
#define C_GRN   "\x1B[32m"
#define C_YEL   "\x1B[33m"
#define C_BLU   "\x1B[34m"
#define C_MAG   "\x1B[35m"
#define C_CYN   "\x1B[36m"
#define C_WHT   "\x1B[37m"
#define C_RESET "\x1B[0m"

int strstarts(const char* str, const char* pre) ;
int strEnds(const char* str, const char* suf) ;
int strEndsNot(const char* str, const char* suf) ;
int isValid(const char* c) ;
void clearFree(void* p, size_t len);
void clearFreeString(char* s);
char* oidc_sprintf(const char* fmt, ...) ;
char* oidc_strcat(const char* str, const char* suf) ;
char* oidc_strcopy(const char* str) ;
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
char* delimitedListToJSONArray(char* str, char delimiter) ;
list_t* delimitedStringToList(char* str, char delimiter) ;
char* listToDelimitedString(list_t* list, char delimiter);
int printError(char* fmt, ...) ;

#endif //OIDC_UTILITIES_H
