#ifndef OIDC_CRYPT_DEF_H
#define OIDC_CRYPT_DEF_H

#include "utils/memory.h"

#include <stddef.h>

struct key_set {
  char* encryption_key;
  char* hash_key;
};

struct cryptParameter {
  size_t nonce_len;
  size_t salt_len;
  size_t mac_len;
  size_t key_len;
  int    base64_variant;
  int    hash_ops_limit;
  int    hash_mem_limit;
  int    hash_alg;
};

struct encryptionInfo {
  char*                 encrypted_base64;
  char*                 nonce_base64;
  char*                 salt_base64;
  char*                 hash_key_base64;
  struct cryptParameter cryptParameter;
};

/**
 * @brief clears and frees an encryptionInfo
 * @param crypt a pointer to the encryptionInfo struct to be cleared
 */
static inline void secFreeEncryptionInfo(struct encryptionInfo* crypt) {
  secFree(crypt->encrypted_base64);
  secFree(crypt->nonce_base64);
  secFree(crypt->salt_base64);
  secFree(crypt->hash_key_base64);
  secFree(crypt);
}

#endif  // OIDC_CRYPT_DEF_H
