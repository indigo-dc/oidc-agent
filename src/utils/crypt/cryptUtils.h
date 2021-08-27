#ifndef CRYPT_UTILS_H
#define CRYPT_UTILS_H

#include "wrapper/list.h"

char* encryptText(const char* text, const char* password);
char* encryptWithVersionLine(const char* text, const char* password);
char* decryptFileContent(const char* fileContent, const char* password);
char* decryptLinesList(list_t* lines, const char* password);

char* randomString(size_t len);

#endif  // CRYPT_UTILS_H
