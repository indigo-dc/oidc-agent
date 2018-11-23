#ifndef CRYPT_H
#define CRYPT_H

#include <sodium.h>

#define KEY_LEN crypto_secretbox_KEYBYTES
#define SALT_LEN crypto_pwhash_SALTBYTES
#define NONCE_LEN crypto_secretbox_NONCEBYTES
#define MAC_LEN crypto_secretbox_MACBYTES

void  initCrypt();
char* crypt_encrypt(
    const unsigned char* text, const char* password,
    char nonce_base64[sodium_base64_ENCODED_LEN(
                          NONCE_LEN, sodium_base64_VARIANT_ORIGINAL) +
                      1],
    char salt_base64[sodium_base64_ENCODED_LEN(SALT_LEN,
                                               sodium_base64_VARIANT_ORIGINAL) +
                     1]);
unsigned char* crypt_decrypt_hex(char* ciphertext, unsigned long cipher_len,
                                 const char* password,
                                 char        nonce_hex[2 * NONCE_LEN + 1],
                                 char        salt_hex[2 * SALT_LEN + 1]);
unsigned char* crypt_decrypt_base64(
    char* ciphertext_base64, unsigned long cipher_len, const char* password,
    char nonce_base64[sodium_base64_ENCODED_LEN(
                          NONCE_LEN, sodium_base64_VARIANT_ORIGINAL) +
                      1],
    char salt_base64[sodium_base64_ENCODED_LEN(SALT_LEN,
                                               sodium_base64_VARIANT_ORIGINAL) +
                     1]);
unsigned char* crypt_keyDerivation_hex(const char* password,
                                       char        salt_hex[2 * SALT_LEN + 1],
                                       int         generateNewSalt);
unsigned char* crypt_keyDerivation_base64(
    const char* password,
    char        salt_base64[sodium_base64_ENCODED_LEN(SALT_LEN,
                                               sodium_base64_VARIANT_ORIGINAL) +
                     1],
    int         generateNewSalt);

#endif  // CRYPT_H
