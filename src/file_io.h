#ifndef FILE_IO_H
#define FILE_IO_H

char* getOidcDir() ;
void writeOidcFile(const char* filename, const char* text) ;
char* readOidcFile(const char* filename) ;
int oidcFileDoesExist(const char* filename) ;
int removeOidcFile(const char* filename) ;

#endif // FILE_IO_H 
