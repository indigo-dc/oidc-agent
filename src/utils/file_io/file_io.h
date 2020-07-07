#ifndef FILE_IO_H
#define FILE_IO_H

#include "list/list.h"
#include "utils/oidc_error.h"

#define OIDC_DIREXIST_OK 1
#define OIDC_DIREXIST_NO 0
#define OIDC_DIREXIST_ERROR -1

oidc_error_t writeFile(const char* filepath, const char* text);
oidc_error_t appendFile(const char* path, const char* text);
char*        readFile(const char* path);
char*        readFILE(FILE* fp);
char*        getLineFromFILE(FILE* fp);
int          fileDoesExist(const char* path);
int          dirExists(const char* path);
oidc_error_t createDir(const char* path);
int          removeFile(const char* path);
list_t*      getLinesFromFile(const char* path);

#endif  // FILE_IO_H
