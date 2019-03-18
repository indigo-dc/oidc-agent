#ifndef CRYPT_UTILS_H
#define CRYPT_UTILS_H

#include "crypt.h"
#include "list/list.h"
#include "utils/oidc_error.h"

char* encryptText(const char* text, const char* password);
char* encryptWithVersionLine(const char* text, const char* password);
char* decryptText(const char* cypher, const char* password,
                  const char* version);
char* decryptFileContent(const char* fileContent, const char* password);
char* decryptLinesList(list_t* lines, const char* password);
int   crypt_compare(const unsigned char* s1, const unsigned char* s2);

char* decryptForIpc(const char*, const unsigned char*);
char* encryptForIpc(const char*, const unsigned char*);

oidc_error_t         lockEncrypt(const char* password);
oidc_error_t         lockDecrypt(const char* password);
struct oidc_account* db_getAccountDecrypted(struct oidc_account* key);
struct oidc_account* db_getAccountDecryptedByShortname(const char* shortname);
void                 db_addAccountEncrypted(struct oidc_account* account);

#endif  // CRYPT_UTILS_H
