#ifndef OIDC_UTILITIES_H
#define OIDC_UTILITIES_H

#include <stddef.h>

int strstarts(const char* str, const char* pre) ;
int isValid(const char* c) ;
void clearFree(void* p, size_t len);
void clearFreeString(char* s);

#endif //OIDC_UTILITIES_H
