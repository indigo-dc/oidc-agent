#ifndef CRYPT_H
#define CRYPT_H

#include <sodium.h>

#define KEY_LEN crypto_secretbox_KEYBYTES
#define SALT_LEN crypto_pwhash_SALTBYTES
#define NONCE_LEN crypto_secretbox_NONCEBYTES
#define MAC_LEN crypto_secretbox_MACBYTES


char* encrypt(const unsigned char* text, const char* password, char nonce_hex[2*NONCE_LEN+1], char salt_hex[2*SALT_LEN+1]) ;
unsigned char* decrypt(char* ciphertext, unsigned long cipher_len, const char* password, char nonce_hex[2*NONCE_LEN+1], char salt_hex[2*SALT_LEN+1]) ;
unsigned char* keyDerivation(const char* password, char salt_hex[2*SALT_LEN+1], int generateNewSalt) ;
// void readNonceSalt(const char* filename) ;
// void writeNonceSalt(const char* filename) ;

#endif //CRYPT_H
