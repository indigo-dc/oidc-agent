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
                                                                  const char* pw_cmd,
                                                                  const char* pw_file,
                                                                  const char* pw_env);
struct oidc_account* getDecryptedAccountFromFilePrompt(const char* accountname,
                                                       const char* pw_cmd,
                                                       const char* pw_file,
                                                       const char* pw_env);
char* getDecryptedAccountAsStringFromFilePrompt(const char* accountname,
                                                const char* pw_cmd,
                                                const char* pw_file,
                                                const char* pw_env);
struct resultWithEncryptionPassword
                     getDecryptedAccountAsStringAndPasswordFromFilePrompt(const char* accountname,
                                                                          const char* pw_cmd,
                                                                          const char* pw_file,
                                                                          const char* pw_env);
struct oidc_account* db_findAccountByShortname(const char* shortname);
list_t*              db_findAccountsByIssuerUrl(const char* issuer_url);

#endif  // ACCOUNT_UTILS_H
