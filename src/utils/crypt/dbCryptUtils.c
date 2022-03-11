#include "dbCryptUtils.h"

#include "crypt.h"
#include "cryptUtils.h"
#include "memoryCrypt.h"
#include "utils/accountUtils.h"
#include "utils/db/account_db.h"
#include "utils/logger.h"
#include "utils/string/stringUtils.h"

/**
 * @brief encrypts sensitive information when the agent is locked.
 * encrypts all loaded access_token, additional encryption (on top of already in
 * place xor) for refresh_token, client_id, client_secret
 * @param loaded the list of currently loaded accounts
 * @param password the lock password that will be used for encryption
 * @return an oidc_error code
 */
oidc_error_t lockEncrypt(const char* password) {
  list_node_t*     node;
  list_iterator_t* it = list_iterator_new(accountDB_getList(), LIST_HEAD);
  while ((node = list_iterator_next(it))) {
    struct oidc_account* acc = node->val;
    char* tmp = encryptText(account_getAccessToken(acc), password);
    if (tmp == NULL) {
      return oidc_errno;
    }
    account_setAccessToken(acc, tmp);
    tmp = encryptText(account_getRefreshToken(acc), password);
    if (tmp == NULL) {
      return oidc_errno;
    }
    account_setRefreshToken(acc, tmp);
    tmp = encryptText(account_getClientId(acc), password);
    if (tmp == NULL) {
      return oidc_errno;
    }
    account_setClientId(acc, tmp);
    if (strValid(account_getClientSecret(acc))) {
      tmp = encryptText(account_getClientSecret(acc), password);
      if (tmp == NULL) {
        return oidc_errno;
      }
      account_setClientSecret(acc, tmp);
    }
  }
  list_iterator_destroy(it);
  return OIDC_SUCCESS;
}

/**
 * @brief decrypts sensitive information when the agent is unlocked.
 * After this call refresh_token, client_id, and client_secret will still be
 * xor encrypted
 * @param loaded the list of currently loaded accounts
 * @param password the lock password that was used for encryption
 * @return an oidc_error code
 */
oidc_error_t lockDecrypt(const char* password) {
  list_node_t*     node;
  list_iterator_t* it = list_iterator_new(accountDB_getList(), LIST_HEAD);
  while ((node = list_iterator_next(it))) {
    struct oidc_account* acc = node->val;
    char* tmp = crypt_decrypt(account_getAccessToken(acc), password);
    if (tmp == NULL) {
      return oidc_errno;
    }
    account_setAccessToken(acc, tmp);
    tmp = crypt_decrypt(account_getRefreshToken(acc), password);
    if (tmp == NULL) {
      return oidc_errno;
    }
    account_setRefreshToken(acc, tmp);
    tmp = crypt_decrypt(account_getClientId(acc), password);
    if (tmp == NULL) {
      return oidc_errno;
    }
    account_setClientId(acc, tmp);
    tmp = crypt_decrypt(account_getClientSecret(acc), password);
    if (tmp == NULL) {
      return oidc_errno;
    }
    account_setClientSecret(acc, tmp);
  }
  list_iterator_destroy(it);
  return OIDC_SUCCESS;
}

struct oidc_account* _db_decryptFoundAccount(struct oidc_account* account) {
  if (account == NULL) {
    return NULL;
  }
  char* tmp = memoryDecrypt(account_getRefreshToken(account));
  if (tmp != NULL) {
    account_setRefreshToken(account, tmp);
  }
  tmp = memoryDecrypt(account_getClientId(account));
  if (tmp != NULL) {
    account_setClientId(account, tmp);
  }
  tmp = memoryDecrypt(account_getClientSecret(account));
  if (tmp != NULL) {
    account_setClientSecret(account, tmp);
  }
  return account;
}

/**
 * @brief finds an account in the list of currently loaded accounts and
 * decryptes the sensitive information
 * @param loaded_accounts the list of currently loaded accounts
 * @param key a key account that should be searched for
 * @return a pointer to the decrypted account
 * @note after usage the account has to be encrypted again by using
 * @c addAccountToList
 */
struct oidc_account* db_getAccountDecrypted(struct oidc_account* key) {
  logger(DEBUG, "Getting / Decrypting account from list");
  struct oidc_account* account = accountDB_findValue(key);
  return _db_decryptFoundAccount(account);
}

struct oidc_account* db_getAccountDecryptedByShortname(const char* shortname) {
  logger(DEBUG, "Getting / Decrypting account from list");
  struct oidc_account* account = db_findAccountByShortname(shortname);
  return _db_decryptFoundAccount(account);
}

/**
 * @brief encrypts the sensitive information of an account and adds it to the
 * list of currently loaded accounts.
 * If there is already a similar account loaded it will be overwritten (removed
 * and the then the new account is added)
 * @param loaded_accounts the list of currently loaded accounts
 * @param account the account that should be added
 */
void db_addAccountEncrypted(struct oidc_account* account) {
  logger(DEBUG, "Adding / Reencrypting account to list");
  account_setRefreshToken(account,
                          memoryEncrypt(account_getRefreshToken(account)));
  account_setClientId(account, memoryEncrypt(account_getClientId(account)));
  account_setClientSecret(account,
                          memoryEncrypt(account_getClientSecret(account)));
  struct oidc_account* found = accountDB_findValue(account);
  if (found != account) {
    if (found) {
      accountDB_removeIfFound(account);
    }
    accountDB_addValue(account);
  }
}
