#ifndef CRYPT_H
#define CRYPT_H

#include "list/list.h"

#include <sodium.h>

struct key_set {
  char* encryption_key;
  char* hash_key;
};

struct cryptParameter;

void           initCrypt();
char*          crypt_encrypt(const char* text, const char* password);
unsigned char* crypt_decrypt_hex(char* ciphertext, unsigned long cipher_len,
                                 const char* password, char nonce_hex[],
                                 char salt_hex[]);
char*          crypt_decrypt(const char* crypt_str, const char* password);
char*          crypt_decryptFromList(list_t* lines, const char* password);
unsigned char* crypt_keyDerivation_hex(const char* password, char salt_hex[],
                                       int                   generateNewSalt,
                                       struct cryptParameter params);
struct key_set crypt_keyDerivation_base64(const char* password,
                                          char        salt_base64[],
                                          int         generateNewSalt,
                                          struct cryptParameter* cryptParams);
char*          toBase64(const char* bin, size_t len);
void           randomFillBase64UrlSafe(char buffer[], size_t buffer_size);

#endif  // CRYPT_H
