#ifndef ACCOUNT_UTILS_H
#define ACCOUNT_UTILS_H

#include "account/account.h"

#include <time.h>

time_t               getMinAccountDeath();
struct oidc_account* getDeathAccount();

struct oidc_account* getAccountFromFile(const char* filepath);
struct oidc_account* getDecryptedAccountFromFile(const char* accountname,
                                                 const char* password);
struct oidc_account* getAccountFromMaybeEncryptedFile(const char* filepath);
struct resultWithEncryptionPassword
                     getDecryptedAccountAndPasswordFromFilePrompt(const char* accountname,
                                                                  const char* pw_cmd);
struct oidc_account* getDecryptedAccountFromFilePrompt(const char* accountname,
                                                       const char* pw_cmd);
char* getDecryptedAccountAsStringFromFilePrompt(const char* accountname,
                                                const char* pw_cmd);
struct resultWithEncryptionPassword
getDecryptedAccountAsStringAndPasswordFromFilePrompt(const char* accountname,
                                                     const char* pw_cmd);

#endif  // ACCOUNT_UTILS_H
