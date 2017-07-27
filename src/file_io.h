#ifndef FILE_IO_H
#define FILE_IO_H

char* readFile(const char* filename) ;
void writeToFile(const char* filename, const char* text) ;
void writeBufferToFile(const char* filename, const char* text, int len) ;

#endif // FILE_IO_H 
