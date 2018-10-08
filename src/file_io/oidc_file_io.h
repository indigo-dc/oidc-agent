#ifndef OIDC_FILE_IO_H
#define OIDC_FILE_IO_H

#include "../../lib/list/src/list.h"
#include "../oidc_error.h"

char*        getOidcDir();
oidc_error_t writeOidcFile(const char* filename, const char* text);
char*        readOidcFile(const char* filename);
int          oidcFileDoesExist(const char* filename);
int          removeOidcFile(const char* filename);
char*        concatToOidcDir(const char* filename);
list_t*      getAccountConfigFileList();
list_t*      getClientConfigFileList();

#endif  // OIDC_FILE_IO_H
