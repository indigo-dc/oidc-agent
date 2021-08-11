#ifndef HEX_CRYPT_H
#define HEX_CRYPT_H

#include "cryptdef.h"

unsigned char* crypt_decrypt_hex(char* ciphertext, unsigned long cipher_len,
                                 const char* password, char nonce_hex[],
                                 char salt_hex[]);
unsigned char* crypt_keyDerivation_hex(const char* password, char salt_hex[],
                                       int                   generateNewSalt,
                                       struct cryptParameter params);
char*          toHex(const unsigned char* bin, size_t bin_len);

#endif  // HEX_CRYPT_H
