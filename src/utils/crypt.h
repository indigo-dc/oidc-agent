#ifndef CRYPT_H
#define CRYPT_H

#include "list/list.h"

#include <sodium.h>

// use these for new encryptions
#define SODIUM_KEY_LEN crypto_secretbox_KEYBYTES
#define SODIUM_SALT_LEN crypto_pwhash_SALTBYTES
#define SODIUM_NONCE_LEN crypto_secretbox_NONCEBYTES
#define SODIUM_MAC_LEN crypto_secretbox_MACBYTES
#define SODIUM_BASE64_VARIANT sodium_base64_VARIANT_ORIGINAL
#define SODIUM_PW_HASH_ALG crypto_pwhash_ALG_DEFAULT
#define SODIUM_PW_HASH_OPSLIMIT crypto_pwhash_OPSLIMIT_INTERACTIVE
#define SODIUM_PW_HASH_MEMLIMIT crypto_pwhash_MEMLIMIT_INTERACTIVE

// for decryption use the stored values
// or for older config files that did not store these values, use the following
// ones
#ifndef HAD_LIBSODIUM23  // if the distro used libsodium23 before 2.1.0 ->
                         // bionic, buster
#define LEG_NONCE_LEN 24
#define LEG_SALT_LEN 16
#define LEG_MAC_LEN 16
#define LEG_KEY_LEN 32
#define LEG_PW_HASH_ALG 1
#define LEG_PW_HASH_OPSLIMIT 4
#define LEG_PW_HASH_MEMLIMIT 33554432
#else  // if the distro used libsodium18 before 2.1.0 -> stretch, xenial
#define LEG_NONCE_LEN 24
#define LEG_SALT_LEN 16
#define LEG_MAC_LEN 16
#define LEG_KEY_LEN 32
#define LEG_PW_HASH_ALG 2
#define LEG_PW_HASH_OPSLIMIT 2
#define LEG_PW_HASH_MEMLIMIT 67108864
#endif

struct key_set {
  char* encryption_key;
  char* hash_key;
};

struct cryptParameter;

void           initCrypt();
char*          crypt_encrypt(const char* text, const char* password);
unsigned char* crypt_decrypt_hex(char* ciphertext, unsigned long cipher_len,
                                 const char* password,
                                 char        nonce_hex[2 * LEG_NONCE_LEN + 1],
                                 char        salt_hex[2 * LEG_SALT_LEN + 1]);
char*          crypt_decrypt(const char* crypt_str, const char* password);
char*          crypt_decryptFromList(list_t* lines, const char* password);
unsigned char* crypt_keyDerivation_hex(const char* password,
                                       char salt_hex[2 * LEG_SALT_LEN + 1],
                                       int  generateNewSalt);
struct key_set crypt_keyDerivation_base64(const char* password,
                                          char        salt_base64[],
                                          int         generateNewSalt,
                                          struct cryptParameter* cryptParams);
char*          toBase64(const char* bin, size_t len);
void           randomFillBase64UrlSafe(char buffer[], size_t buffer_size);

#endif  // CRYPT_H
