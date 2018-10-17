#include "cryptUtils.h"
#include "../crypt.h"
#include "../oidc_error.h"
#include "memory.h"

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

void secFreeHashed(struct hashed* h) {
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
