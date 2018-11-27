#ifndef CRYPT_H
#define CRYPT_H

#include <sodium.h>

// #define KEY_LEN crypto_secretbox_KEYBYTES
#define SALT_LEN crypto_pwhash_SALTBYTES
#define NONCE_LEN crypto_secretbox_NONCEBYTES
#define MAC_LEN crypto_secretbox_MACBYTES

#ifndef HAD_LIBSODIUM23
#define KEY_LEN 32
#define PW_HASH_ALG 1
#define PW_HASH_OPS_LIMIT 4
#define PW_HASH_MEMLIMIT 33554432
#else
#define KEY_LEN 32
#define PW_HASH_ALG 2
#define PW_HASH_OPS_LIMIT 2
#define PW_HASH_MEMLIMIT 67108864
#endif

struct key_set {
  char* encryption_key;
  char* hash_key;
};

void  initCrypt();
char* crypt_encrypt(
    const unsigned char* text, const char* password,
    char nonce_base64[sodium_base64_ENCODED_LEN(
                          NONCE_LEN, sodium_base64_VARIANT_ORIGINAL) +
                      1],
    char key_str[crypto_pwhash_STRBYTES]);
unsigned char* crypt_decrypt_hex(char* ciphertext, unsigned long cipher_len,
                                 const char* password,
                                 char        nonce_hex[2 * NONCE_LEN + 1],
                                 char        salt_hex[2 * SALT_LEN + 1]);
unsigned char* crypt_decrypt_base64(
    char* ciphertext_base64, unsigned long cipher_len, const char* password,
    char nonce_base64[sodium_base64_ENCODED_LEN(
                          NONCE_LEN, sodium_base64_VARIANT_ORIGINAL) +
                      1],
    char key_str[crypto_pwhash_STRBYTES]);
unsigned char* crypt_keyDerivation_hex(const char* password,
                                       char        salt_hex[2 * SALT_LEN + 1],
                                       int         generateNewSalt);
struct key_set crypt_keyDerivation_base64(
    const char* password,
    char        salt_base64[sodium_base64_ENCODED_LEN(SALT_LEN,
                                               sodium_base64_VARIANT_ORIGINAL) +
                     1],
    int         generateNewSalt);
char* toBase64(const char* bin, size_t len);
void  randomFillBase64UrlSafe(char buffer[], size_t buffer_size);

#endif  // CRYPT_H
