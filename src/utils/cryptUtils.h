#ifndef CRYPT_UTILS_H
#define CRYPT_UTILS_H

#include "crypt.h"
#include "list/list.h"

char*          encryptText(const char* text, const char* password);
unsigned char* decryptText(const char* cypher, const char* password,
                           const char* version);
unsigned char* decryptOidcFile(const char* filename, const char* password);
unsigned char* decryptFile(const char* filepath, const char* password);
unsigned char* decryptFileContent(const char* fileContent,
                                  const char* password);
unsigned char* decryptLinesList(list_t* lines, const char* password);
int            crypt_compare(const unsigned char* s1, const unsigned char* s2);

char* hashPassword(const char* pw);
int   verify_hashedPassword(const char* pw, const char* hash_str);
void  lockEncrypt(list_t* loaded, const char* password);
void  lockDecrypt(list_t* loaded, const char* password);
struct oidc_account* getAccountFromList(list_t*              loaded_accounts,
                                        struct oidc_account* key);
void addAccountToList(list_t* loaded_accounts, struct oidc_account* account);

#ifndef secFreeHashed
#define secFreeHashed(ptr) \
  do {                     \
    _secFreeHashed((ptr)); \
    (ptr) = NULL;          \
  } while (0)
#endif  // secFreeHased

#endif  // CRYPT_UTILS_H
