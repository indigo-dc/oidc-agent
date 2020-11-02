#ifndef DB_CRYPT_UTILS_H
#define DB_CRYPT_UTILS_H

#include "account/account.h"
#include "utils/oidc_error.h"

oidc_error_t lockEncrypt(const char* password);
oidc_error_t lockDecrypt(const char* password);

struct oidc_account* _db_decryptFoundAccount(struct oidc_account* account);
struct oidc_account* db_getAccountDecrypted(struct oidc_account* key);
struct oidc_account* db_getAccountDecryptedByShortname(const char* shortname);
void                 db_addAccountEncrypted(struct oidc_account* account);

#endif  // DB_CRYPT_UTILS_H
