#ifndef FILE_IO_H
#define FILE_IO_H

#include "../../lib/list/src/list.h"
#include "../oidc_error.h"

oidc_error_t writeFile(const char* filepath, const char* text);
char*        readFile(const char* path);
int          fileDoesExist(const char* path);
int          dirExists(const char* path);
int          removeFile(const char* path);
list_t*      getLinesFromFile(const char* path);

#endif  // FILE_IO_H
