#ifndef CRYPT_UTILS_H
#define CRYPT_UTILS_H

#include "../../lib/list/src/list.h"
#include "../crypt.h"

struct hashed {
  unsigned char* hash;
  char           salt_hex[2 * SALT_LEN + 1];
};

char*          encryptText(const char* text, const char* password);
unsigned char* decryptText(const char* cypher, const char* password);
int            crypt_compare(const unsigned char* s1, const unsigned char* s2);
struct hashed* hash(const char* str);
int            compareToHash(const char* str, struct hashed* h);
void           _secFreeHashed(struct hashed* h);

void                 lockEncrypt(list_t* loaded, const char* password);
void                 lockDecrypt(list_t* loaded, const char* password);
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
