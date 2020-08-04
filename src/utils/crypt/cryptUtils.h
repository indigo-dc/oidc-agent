#ifndef CRYPT_UTILS_H
#define CRYPT_UTILS_H

#include "list/list.h"

char* encryptText(const char* text, const char* password);
char* encryptWithVersionLine(const char* text, const char* password);
char* decryptText(const char* cypher, const char* password,
                  const char* version);
char* decryptFileContent(const char* fileContent, const char* password);
char* decryptLinesList(list_t* lines, const char* password);
int   crypt_compare(const unsigned char* s1, const unsigned char* s2);

char* randomString(size_t len);

#endif  // CRYPT_UTILS_H
