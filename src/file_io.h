#ifndef FILE_IO_H
#define FILE_IO_H

char* readFile(const char* filename) ;
void writeToFile(const char* filename, const char* text) ;
void writeBufferToFile(const char* filename, const char* text, int len) ;
char* getOidcDir() ;
void writeOidcFile(const char* filename, const char* text) ;
char* readOidcFile(const char* filename) ;
int oidcFileDoesExist(const char* filename) ;

#endif // FILE_IO_H 
