#ifndef FILE_IO_H
#define FILE_IO_H

#include <stdio.h>
#include <sys/types.h>

#include "defines/msys.h"
#include "utils/oidc_error.h"
#include "wrapper/list.h"

#define OIDC_DIREXIST_OK 1
#define OIDC_DIREXIST_NO 0
#define OIDC_DIREXIST_ERROR -1

#define DEFAULT_COMMENT_CHAR '#'

oidc_error_t writeFile(const char* filepath, const char* text);
oidc_error_t appendFile(const char* path, const char* text);
char*        readFile(const char* path);
char*        readFILE(FILE* fp);
oidc_error_t readBinaryFile(const char* path, char** buffer, size_t* size);
char*        getLineFromFILE(FILE* fp);
char*        getLineFromFile(const char* path);
int          fileDoesExist(const char* path);
int          dirExists(const char* path);
oidc_error_t createDir(const char* path);
int          removeFile(const char* path);
list_t*      getLinesFromFile(const char* path);
list_t*      getLinesFromFileWithoutComments(const char* path);
char* getFileContentAfterLine(const char* path, const char* lineContentPrefix,
                              long startChar, unsigned char allIfLineNotFound);
oidc_error_t mkpath(const char* p, mode_t mode);
char*        getExistingLocation(list_t* possibleLocations);

#ifdef MINGW
int getline(char** lineptr, size_t* n, FILE* stream);
#endif

#endif  // FILE_IO_H
