#ifndef CRYPT_FILE_UTILS_H
#define CRYPT_FILE_UTILS_H

#include "utils/oidc_error.h"

oidc_error_t encryptAndWriteToFile(const char* text, const char* filepath,
                                   const char* password);
oidc_error_t encryptAndWriteToOidcFile(const char* text, const char* filename,
                                       const char* password);
char*        decryptFile(const char* filepath, const char* password);
char*        decryptOidcFile(const char* filename, const char* password);

#endif  // CRYPT_FILE_UTILS_H
