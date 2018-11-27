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

char* decryptOidcFile(const char* filename, const char* password) {
  char* filepath = concatToOidcDir(filename);
  char* ret      = decryptFile(filepath, password);
  secFree(filepath);
  return ret;
}

char* decryptFile(const char* filepath, const char* password) {
  list_t* lines = getLinesFromFile(filepath);
  char*   ret   = decryptLinesList(lines, password);
  list_destroy(lines);
  return ret;
}

char* decryptFileContent(const char* fileContent, const char* password) {
  list_t* lines = delimitedStringToList(fileContent, '\n');
  char*   ret   = decryptLinesList(lines, password);
  list_destroy(lines);
  return ret;
}

char* decryptLinesList(list_t* lines, const char* password) {
  list_node_t* node   = list_at(lines, 0);
  char*        cipher = node ? node->val : NULL;
  node                = list_at(lines, 1);
  char* version_line  = node ? node->val : NULL;
  char* version       = versionLineToSimpleVersion(version_line);
  char* ret           = decryptText(cipher, password, version);
  secFree(version);
  return ret;
}

char* decryptText(const char* cipher, const char* password,
                  const char* version) {
  if (cipher == NULL || password == NULL || version == NULL) {
    oidc_setArgNullFuncError(__func__);
    return NULL;
  }
  if (versionAtLeast(version, MIN_BASE64_VERSION)) {
    return crypt_decrypt(cipher, password);

  } else {  // old config file format; using hex encoding
    char*          fileText       = oidc_strcopy(cipher);
    unsigned long  cipher_len     = atoi(strtok(fileText, ":"));
    char*          salt_encoded   = strtok(NULL, ":");
    char*          nonce_encoded  = strtok(NULL, ":");
    char*          cipher_encoded = strtok(NULL, ":");
    unsigned char* decrypted      = crypt_decrypt_hex(
        cipher_encoded, cipher_len, password, nonce_encoded, salt_encoded);
    secFree(fileText);
    return (char*)decrypted;
  }
}

/**
 *
 * @note before version 2.1.0 this function used hex encoding
 */
char* encryptText(const char* text, const char* password) {
  return crypt_encrypt(text, password);  // TODO version in file
}

// char* hashPassword(const char* pw) {
//   if (pw == NULL) {
//     oidc_setArgNullFuncError(__func__);
//     return NULL;
//   }
//   char* useStr = oidc_sprintf(
//       "%s%s", pw,
//       pw);  // hashPassword and verify_hashPassword hash a string derived
//       from
//             // the original string. This way the hashed value is
//             // different from the key used for encryption
//   char* hash_str = secAlloc(KEY_STR_LEN);
//   if (hash(hash_str, useStr) != OIDC_SUCCESS) {
//     secFree(useStr);
//     secFree(hash_str);
//     return NULL;
//   }
//   secFree(useStr);
//   return hash_str;
// }
//
// int verify_hashedPassword(const char* pw, const char* hash_str) {
//   if (pw == NULL || hash_str == NULL) {
//     oidc_setArgNullFuncError(__func__);
//     return -1;
//   }
//   char* useStr = oidc_sprintf("%s%s", pw, pw);
//   int   ret    = hash_verify(hash_str, useStr);
//   secFree(useStr);
//   return ret;
// }

// int crypt_compare(const unsigned char* s1, const unsigned char* s2) {
//   int    m = 0;
//   size_t i = 0;
//   size_t j = 0;
//   size_t k = 0;
//
//   if (s1 == NULL || s2 == NULL)
//     return 0;
//
//   while (1) {
//     m |= s1[i] ^ s2[j];
//
//     if (s1[i] == '\0') {
//       break;
//     }
//     i++;
//
//     if (s2[j] != '\0') {
//       j++;
//     }
//     if (s2[j] == '\0') {
//       k++;
//     }
//   }
//
//   return m == 0;
// }
//
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
