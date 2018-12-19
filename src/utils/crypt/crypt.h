#ifndef CRYPT_H
#define CRYPT_H

#include "cryptdef.h"
#include "list/list.h"

void                   initCrypt();
char*                  crypt_encrypt(const char* text, const char* password);
struct encryptionInfo* crypt_encryptWithKey(const unsigned char* text,
                                            const unsigned char* key);
char*          crypt_decrypt(const char* crypt_str, const char* password);
char*          crypt_decryptFromList(list_t* lines, const char* password);
unsigned char* crypt_decryptWithKey(const struct encryptionInfo* crypt,
                                    unsigned long                cipher_len,
                                    const unsigned char*         key);

struct key_set crypt_keyDerivation_base64(const char* password,
                                          char        salt_base64[],
                                          int         generateNewSalt,
                                          struct cryptParameter* cryptParams);
char*          toBase64(const char* bin, size_t len);
int  fromBase64(const char* base64, size_t bin_len, unsigned char* bin);
void randomFillBase64UrlSafe(char buffer[], size_t buffer_size);
struct cryptParameter newCryptParameters();

#endif  // CRYPT_H
