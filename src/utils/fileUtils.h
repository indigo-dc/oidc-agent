#ifndef FILE_UTILS_H
#define FILE_UTILS_H

void           assertOidcDirExists();
unsigned char* decryptFileContent(const char* fileContent,
                                  const char* password);

#endif  // FILE_UTILS_H
