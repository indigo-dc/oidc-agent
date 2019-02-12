#ifndef FILE_UTILS_H
#define FILE_UTILS_H

#include "list/list.h"

void assertOidcDirExists();
void checkOidcDirExists();

list_t* getAccountConfigFileList();
list_t* getClientConfigFileList();

int compareFilesByName(const char* filename1, const char* filename2);
int compareOidcFilesByDateModified(const char* filename1,
                                   const char* filename2);
int compareOidcFilesByDateAccessed(const char* filename1,
                                   const char* filename2);

char* generateClientConfigFileName(const char* issuer_url,
                                   const char* client_id);

#endif  // FILE_UTILS_H
