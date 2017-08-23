#ifndef FILE_IO_H
#define FILE_IO_H

#include "oidc_error.h"

char* getOidcDir() ;
oidc_error_t writeOidcFile(const char* filename, const char* text) ;
char* readOidcFile(const char* filename) ;
char* readFile(const char* path);
int fileDoesExist(const char* path);
int oidcFileDoesExist(const char* filename) ;
int removeOidcFile(const char* filename) ;

#endif // FILE_IO_H 
