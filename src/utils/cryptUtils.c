#include "cryptUtils.h"
#include "account/account.h"
#include "crypt.h"
#include "list/list.h"
#include "memory.h"
#include "memoryCrypt.h"
#include "oidc_error.h"
#include "utils/listUtils.h"

#include <stdlib.h>
#include <string.h>

unsigned char* decryptText(const char* cipher, const char* password) {
  if (cipher == NULL) {
    oidc_setArgNullFuncError(__func__);
    return NULL;
  }
  char*          fileText   = oidc_strcopy(cipher);
  unsigned long  cipher_len = atoi(strtok(fileText, ":"));
  char*          salt_hex   = strtok(NULL, ":");
  char*          nonce_hex  = strtok(NULL, ":");
  char*          cipher_tex = strtok(NULL, ":");
  unsigned char* decrypted =
      crypt_decrypt(cipher_tex, cipher_len, password, nonce_hex, salt_hex);
  secFree(fileText);
  return decrypted;
}

char* encryptText(const char* text, const char* password) {
  char          salt_hex[2 * SALT_LEN + 1]   = {0};
  char          nonce_hex[2 * NONCE_LEN + 1] = {0};
  unsigned long cipher_len                   = strlen(text) + MAC_LEN;
  char*         cipher_hex =
      crypt_encrypt((unsigned char*)text, password, nonce_hex, salt_hex);
  char* fmt    = "%lu:%s:%s:%s";
  char* cipher = oidc_sprintf(fmt, cipher_len, salt_hex, nonce_hex, cipher_hex);
  secFree(cipher_hex);
  return cipher;
}

void _secFreeHashed(struct hashed* h) {
  if (h == NULL) {
    return;
  }
  secFree(h->hash);
  secFree(h);
}

struct hashed* hash(const char* str) {
  struct hashed* h      = secAlloc(sizeof(struct hashed));
  char*          useStr = oidc_sprintf(
      "%s%s", str, str);  // hash and compareToHash hash a string derived from
                          // the original string. This way the hashed value is
                          // different from the key used for encryption
  h->hash = crypt_keyDerivation(useStr, h->salt_hex, 1);
  secFree(useStr);
  return h;
}

int compareToHash(const char* str, struct hashed* h) {
  char*          useStr = oidc_sprintf("%s%s", str, str);
  unsigned char* hashed = crypt_keyDerivation(useStr, h->salt_hex, 0);
  secFree(useStr);
  int ret = crypt_compare(hashed, h->hash);
  secFree(hashed);
  return ret;
}

int crypt_compare(const unsigned char* s1, const unsigned char* s2) {
  int    m = 0;
  size_t i = 0;
  size_t j = 0;
  size_t k = 0;

  if (s1 == NULL || s2 == NULL)
    return 0;

  while (1) {
    m |= s1[i] ^ s2[j];

    if (s1[i] == '\0') {
      break;
    }
    i++;

    if (s2[j] != '\0') {
      j++;
    }
    if (s2[j] == '\0') {
      k++;
    }
  }

  return m == 0;
}

/*
 * encrypts all loaded access_token, additional encryption (on top of already in
 * place xor) for refresh_token, client_id, client_secret
 */
void lockEncrypt(list_t* loaded, const char* password) {
  list_node_t*     node;
  list_iterator_t* it = list_iterator_new(loaded, LIST_HEAD);
  while ((node = list_iterator_next(it))) {
    struct oidc_account* acc = node->val;
    account_setAccessToken(acc,
                           encryptText(account_getAccessToken(*acc), password));
    account_setRefreshToken(
        acc, encryptText(account_getRefreshToken(*acc), password));
    account_setClientId(acc, encryptText(account_getClientId(*acc), password));
    account_setClientSecret(
        acc, encryptText(account_getClientSecret(*acc), password));
  }
  list_iterator_destroy(it);
}

void lockDecrypt(list_t* loaded, const char* password) {
  list_node_t*     node;
  list_iterator_t* it = list_iterator_new(loaded, LIST_HEAD);
  while ((node = list_iterator_next(it))) {
    struct oidc_account* acc = node->val;
    account_setAccessToken(
        acc, (char*)decryptText(account_getAccessToken(*acc), password));
    account_setRefreshToken(
        acc, (char*)decryptText(account_getRefreshToken(*acc), password));
    account_setClientId(
        acc, (char*)decryptText(account_getClientId(*acc), password));
    account_setClientSecret(
        acc, (char*)decryptText(account_getClientSecret(*acc), password));
  }
  list_iterator_destroy(it);
}

struct oidc_account* getAccountFromList(list_t*              loaded_accounts,
                                        struct oidc_account* key) {
  list_node_t*         node = findInList(loaded_accounts, key);
  struct oidc_account* account;
  if (node == NULL || (account = node->val) == NULL) {
    return NULL;
  }
  account_setRefreshToken(account,
                          memoryDecrypt(account_getRefreshToken(*account)));
  account_setClientId(account, memoryDecrypt(account_getClientId(*account)));
  account_setClientSecret(account,
                          memoryDecrypt(account_getClientSecret(*account)));
  return account;
}

void addAccountToList(list_t* loaded_accounts, struct oidc_account* account) {
  account_setRefreshToken(account,
                          memoryEncrypt(account_getRefreshToken(*account)));
  account_setClientId(account, memoryEncrypt(account_getClientId(*account)));
  account_setClientSecret(account,
                          memoryEncrypt(account_getClientSecret(*account)));
  list_node_t* node = findInList(loaded_accounts, account);
  if (node && node->val != account) {
    list_remove(loaded_accounts, node);
  }
  if (NULL == node || node->val != account) {
    list_rpush(loaded_accounts, list_node_new(account));
  }
}
