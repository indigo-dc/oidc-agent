#ifndef CRYPT_H
#define CRYPT_H

#include <sodium.h>

#define KEY_LEN crypto_secretbox_KEYBYTES
#define SALT_LEN crypto_pwhash_SALTBYTES
#define NONCE_LEN crypto_secretbox_NONCEBYTES
#define MAC_LEN crypto_secretbox_MACBYTES

unsigned char* encrypt(const unsigned char* text, const char* password) ;
unsigned char* decrypt(const unsigned char* ciphertext, const char* password) ;
unsigned char* keyDerivation(const char* password) ;
void readNonceSalt(const char* filename) ;
void writeNonceSalt(const char* filename) ;

#endif //CRYPT_H
