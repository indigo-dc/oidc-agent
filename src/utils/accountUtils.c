#include "accountUtils.h"
#include "account/account.h"
#include "deathUtils.h"
#include "utils/crypt/cryptUtils.h"
#include "utils/db/account_db.h"
#include "utils/file_io/cryptFileUtils.h"
#include "utils/file_io/file_io.h"
#include "utils/file_io/promptCryptFileUtils.h"
#include "utils/json.h"
#include "utils/logger.h"
#include "utils/promptUtils.h"
#include "utils/stringUtils.h"

#include <time.h>

/**
 * @brief returns the minimum death time in an account list
 * @param accounts a list of (loaded) accounts
 * @return the minimum time of death; might be @c 0
 */
time_t getMinAccountDeath() {
  logger(DEBUG, "Getting min death time for accounts");
  return accountDB_getMinDeath((time_t(*)(void*))account_getDeath);
}

/**
 * @brief returns an account that death was prior to the current time
 * @param accounts a list of (loaded) accounts - searchspace
 * only one death account is returned per call; to find all death accounts in @p
 * accounts @c getDeathAccount should be called until it returns @c NULL
 * @return a pointer to a dead account or @c NULL
 */
struct oidc_account* getDeathAccount() {
  logger(DEBUG, "Searching for death accounts");
  return accountDB_getDeathEntry((time_t(*)(void*))account_getDeath);
}

struct oidc_account* getAccountFromMaybeEncryptedFile(const char* filepath) {
  if (filepath == NULL) {
    oidc_setArgNullFuncError(__func__);
    return NULL;
  }
  char* config = readFile(filepath);
  if (NULL == config) {
    return NULL;
  }
  if (!isJSONObject(config)) {
    char* tmp = getDecryptedTextWithPromptFor(
        config, filepath, decryptFileContent, 0, NULL, NULL, NULL);
    if (NULL == tmp) {
      return NULL;
    }
    secFree(config);
    config = tmp;
  }
  struct oidc_account* p = getAccountFromJSON(config);
  secFree(config);
  return p;
}

struct oidc_account* getAccountFromFile(const char* filepath) {
  if (filepath == NULL) {
    oidc_setArgNullFuncError(__func__);
    return NULL;
  }
  char* config = readFile(filepath);
  if (NULL == config) {
    return NULL;
  }
  struct oidc_account* p = getAccountFromJSON(config);
  secFree(config);
  return p;
}

/**
 * @brief reads the encrypted configuration for a given short name and decrypts
 * the configuration.
 * @param accountname the short name of the account that should be decrypted
 * @param password the encryption password
 * @return a pointer to an oidc_account. Has to be freed after usage. Null on
 * failure.
 */
struct oidc_account* getDecryptedAccountFromFile(const char* accountname,
                                                 const char* password) {
  if (accountname == NULL || password == NULL) {
    oidc_setArgNullFuncError(__func__);
    return NULL;
  }
  char* decrypted = decryptOidcFile(accountname, password);
  if (NULL == decrypted) {
    return NULL;
  }
  struct oidc_account* p = getAccountFromJSON(decrypted);
  secFree(decrypted);
  return p;
}

/**
 * @brief reads the encrypted configuration for a given short name and decrypts
 * the configuration.
 * @param accountname the short name of the account that should be decrypted
 * @param pw_cmd  the command used to get the encryption password, can be
 * @c NULL
 * @param pw_file a filepath used to get the encryption password, can be
 * @c NULL
 * @return a pointer to an oidc_account. Has to be freed after usage. Null on
 * failure.
 */
struct resultWithEncryptionPassword
getDecryptedAccountAndPasswordFromFilePrompt(const char* accountname,
                                             const char* pw_cmd,
                                             const char* pw_file,
                                             const char* pw_env) {
  if (accountname == NULL) {
    oidc_setArgNullFuncError(__func__);
    return RESULT_WITH_PASSWORD_NULL;
  }
  struct resultWithEncryptionPassword result =
      getDecryptedOidcFileAndPasswordFor(accountname, pw_cmd, pw_file, pw_env);
  char* config = result.result;
  if (NULL == config) {
    return result;
  }
  struct oidc_account* p = getAccountFromJSON(config);
  secFree(config);
  result.result = p;
  return result;
}

struct oidc_account* getDecryptedAccountFromFilePrompt(const char* accountname,
                                                       const char* pw_cmd,
                                                       const char* pw_file,
                                                       const char* pw_env) {
  if (accountname == NULL) {
    oidc_setArgNullFuncError(__func__);
    return NULL;
  }
  char* config = getDecryptedOidcFileFor(accountname, pw_cmd, pw_file, pw_env);
  if (NULL == config) {
    return NULL;
  }
  struct oidc_account* p = getAccountFromJSON(config);
  secFree(config);
  return p;
}

struct resultWithEncryptionPassword
getDecryptedAccountAsStringAndPasswordFromFilePrompt(const char* accountname,
                                                     const char* pw_cmd,
                                                     const char* pw_file,
                                                     const char* pw_env) {
  if (accountname == NULL) {
    oidc_setArgNullFuncError(__func__);
    return RESULT_WITH_PASSWORD_NULL;
  }
  struct resultWithEncryptionPassword result =
      getDecryptedAccountAndPasswordFromFilePrompt(accountname, pw_cmd, pw_file,
                                                   pw_env);
  if (NULL == result.result) {
    return result;
  }
  char* json = accountToJSONString(result.result);
  secFreeAccount(result.result);
  result.result = json;
  return result;
}

char* getDecryptedAccountAsStringFromFilePrompt(const char* accountname,
                                                const char* pw_cmd,
                                                const char* pw_file,
                                                const char* pw_env) {
  if (accountname == NULL) {
    oidc_setArgNullFuncError(__func__);
    return NULL;
  }
  struct resultWithEncryptionPassword result =
      getDecryptedAccountAsStringAndPasswordFromFilePrompt(accountname, pw_cmd,
                                                           pw_file, pw_env);
  secFree(result.password);
  return result.result;
}

struct oidc_account* db_findAccountByShortname(const char* shortname) {
  if (shortname == NULL) {
    return NULL;
  }
  char*                tmp     = oidc_strcopy(shortname);
  struct oidc_account  key     = {.shortname = tmp};
  struct oidc_account* account = accountDB_findValue(&key);
  secFree(tmp);
  return account;
}

list_t* db_findAccountsByIssuerUrl(const char* issuer_url) {
  if (issuer_url == NULL) {
    return NULL;
  }
  matchFunction oldMatch =
      accountDB_setMatchFunction((matchFunction)account_matchByIssuerUrl);
  char*               tmp      = oidc_strcopy(issuer_url);
  struct oidc_issuer  iss      = {.issuer_url = tmp};
  struct oidc_account key      = {.issuer = &iss};
  list_t*             accounts = accountDB_findAllValues(&key);
  secFree(tmp);
  accountDB_setMatchFunction(oldMatch);
  return accounts;
}
