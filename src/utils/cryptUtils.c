#include "cryptUtils.h"
#include "account/account.h"
#include "crypt.h"
#include "list/list.h"
#include "memory.h"
#include "memoryCrypt.h"
#include "oidc_error.h"
#include "utils/file_io/file_io.h"
#include "utils/file_io/oidc_file_io.h"
#include "utils/listUtils.h"
#include "utils/versionUtils.h"
#include "version.h"

#include <stdlib.h>
#include <string.h>

unsigned char* decryptOidcFile(const char* filename, const char* password) {
  char*          filepath = concatToOidcDir(filename);
  unsigned char* ret      = decryptFile(filepath, password);
  secFree(filepath);
  return ret;
}

unsigned char* decryptFile(const char* filepath, const char* password) {
  list_t*        lines = getLinesFromFile(filepath);
  unsigned char* ret   = decryptLinesList(lines, password);
  list_destroy(lines);
  return ret;
}

unsigned char* decryptFileContent(const char* fileContent,
                                  const char* password) {
  list_t*        lines = delimitedStringToList(fileContent, '\n');
  unsigned char* ret   = decryptLinesList(lines, password);
  list_destroy(lines);
  return ret;
}

unsigned char* decryptLinesList(list_t* lines, const char* password) {
  list_node_t* node           = list_at(lines, 0);
  char*        cipher         = node ? node->val : NULL;
  node                        = list_at(lines, 1);
  char*          version_line = node ? node->val : NULL;
  char*          version      = versionLineToSimpleVersion(version_line);
  unsigned char* ret          = decryptText(cipher, password, version);
  secFree(version);
  return ret;
}

unsigned char* decryptText(const char* cipher, const char* password,
                           const char* version) {
  if (cipher == NULL || password == NULL || version == NULL) {
    oidc_setArgNullFuncError(__func__);
    return NULL;
  }
  char*         fileText       = oidc_strcopy(cipher);
  unsigned long cipher_len     = atoi(strtok(fileText, ":"));
  char*         salt_encoded   = strtok(NULL, ":");
  char*         nonce_encoded  = strtok(NULL, ":");
  char*         cipher_encoded = strtok(NULL, ":");
  unsigned char* (*decrypter)(char*, unsigned long, const char*, char[],
                              char[]) =
      versionAtLeast(version, MIN_BASE64_VERSION) ? crypt_decrypt_base64
                                                  : crypt_decrypt_hex;
  unsigned char* decrypted = decrypter(cipher_encoded, cipher_len, password,
                                       nonce_encoded, salt_encoded);
  secFree(fileText);
  return decrypted;
}

/**
 *
 * @note before version 2.1.0 this function used hex encoding
 */
char* encryptText(const char* text, const char* password) {
  char          nonce_base64[sodium_base64_ENCODED_LEN(NONCE_LEN,
                                              sodium_base64_VARIANT_ORIGINAL) +
                    1]          = {0};
  char          key_str[KEY_STR_LEN + 1] = {0};
  unsigned long cipher_len               = strlen(text) + MAC_LEN;
  char*         cipher_base64 =
      crypt_encrypt((unsigned char*)text, password, nonce_base64, key_str);
  // Current config file format:
  // 1 cipher_len
  // 2 nonce_base64
  // 3 key_str
  // 4 cipher_base64
  char* fmt = "%lu\n%s\n%s\n%s";
  char* cipher =
      oidc_sprintf(fmt, cipher_len, nonce_base64, key_str, cipher_base64);
  secFree(cipher_base64);
  moresecure_memzero(key_str, KEY_STR_LEN);
  return cipher;
}

char* hashPassword(const char* pw) {
  if (pw == NULL) {
    oidc_setArgNullFuncError(__func__);
    return NULL;
  }
  char* useStr = oidc_sprintf(
      "%s%s", pw,
      pw);  // hashPassword and verify_hashPassword hash a string derived from
            // the original string. This way the hashed value is
            // different from the key used for encryption
  char* hash_str = secAlloc(KEY_STR_LEN);
  if (hash(hash_str, useStr) != OIDC_SUCCESS) {
    secFree(useStr);
    secFree(hash_str);
    return NULL;
  }
  secFree(useStr);
  return hash_str;
}

int verify_hashedPassword(const char* pw, const char* hash_str) {
  if (pw == NULL || hash_str == NULL) {
    oidc_setArgNullFuncError(__func__);
    return -1;
  }
  char* useStr = oidc_sprintf("%s%s", pw, pw);
  int   ret    = hash_verify(hash_str, useStr);
  secFree(useStr);
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
        acc,
        (char*)decryptText(account_getAccessToken(*acc), password,
                           VERSION));  // This is just in memory enrcyption, so
                                       // version is the current version
    account_setRefreshToken(
        acc,
        (char*)decryptText(account_getRefreshToken(*acc), password, VERSION));
    account_setClientId(
        acc, (char*)decryptText(account_getClientId(*acc), password, VERSION));
    account_setClientSecret(
        acc,
        (char*)decryptText(account_getClientSecret(*acc), password, VERSION));
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
