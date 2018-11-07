#ifndef FILE_UTILS_H
#define FILE_UTILS_H

void           assertOidcDirExists();
void           checkOidcDirExists();
unsigned char* decryptFileContent(const char* fileContent,
                                  const char* password);

#endif  // FILE_UTILS_H
