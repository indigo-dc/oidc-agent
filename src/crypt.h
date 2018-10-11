#ifndef CRYPT_H
#define CRYPT_H

#include <sodium.h>

#define KEY_LEN crypto_secretbox_KEYBYTES
#define SALT_LEN crypto_pwhash_SALTBYTES
#define NONCE_LEN crypto_secretbox_NONCEBYTES
#define MAC_LEN crypto_secretbox_MACBYTES

void           initCrypt();
char*          crypt_encrypt(const unsigned char* text, const char* password,
                             char nonce_hex[2 * NONCE_LEN + 1],
                             char salt_hex[2 * SALT_LEN + 1]);
unsigned char* crypt_decrypt(char* ciphertext, unsigned long cipher_len,
                             const char* password,
                             char        nonce_hex[2 * NONCE_LEN + 1],
                             char        salt_hex[2 * SALT_LEN + 1]);
unsigned char* crypt_keyDerivation(const char* password,
                                   char        salt_hex[2 * SALT_LEN + 1],
                                   int         generateNewSalt);

char* getRandomHexString(size_t size);
void  randomFillHex(char buffer[], size_t buffer_size);
#endif  // CRYPT_H
