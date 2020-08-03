#include "hexCrypt.h"
#include "utils/logger.h"
#include "utils/oidc_error.h"
#include "utils/printer.h"

#include <sodium.h>
#include <string.h>

// if the distro used libsodium18 before 2.1.0 -> stretch, xenial
#define LEG18_NONCE_LEN 24
#define LEG18_SALT_LEN 16
#define LEG18_MAC_LEN 16
#define LEG18_KEY_LEN 32
#define LEG18_PW_HASH_ALG 1
#define LEG18_PW_HASH_OPSLIMIT 4
#define LEG18_PW_HASH_MEMLIMIT 33554432
// if the distro used libsodium23 before 2.1.0 -> bionic, buster
#define LEG23_NONCE_LEN 24
#define LEG23_SALT_LEN 16
#define LEG23_MAC_LEN 16
#define LEG23_KEY_LEN 32
#define LEG23_PW_HASH_ALG 2
#define LEG23_PW_HASH_OPSLIMIT 2
#define LEG23_PW_HASH_MEMLIMIT 67108864

/**
 * Legacy crypt parameters for file encrypted before 2.1.0 on a system that used
 * libsodium23 (bionic, buster)
 */
static struct cryptParameter legacy_23_cryptParams = {LEG23_NONCE_LEN,
                                                      LEG23_SALT_LEN,
                                                      LEG23_MAC_LEN,
                                                      LEG23_KEY_LEN,
                                                      0,
                                                      LEG23_PW_HASH_OPSLIMIT,
                                                      LEG23_PW_HASH_MEMLIMIT,
                                                      LEG23_PW_HASH_ALG};
/**
 * Legacy crypt parameters for file encrypted before 2.1.0 on a system that used
 * libsodium28 (xenial, stretch)
 */
static struct cryptParameter legacy_18_cryptParams = {LEG18_NONCE_LEN,
                                                      LEG18_SALT_LEN,
                                                      LEG18_MAC_LEN,
                                                      LEG18_KEY_LEN,
                                                      0,
                                                      LEG18_PW_HASH_OPSLIMIT,
                                                      LEG18_PW_HASH_MEMLIMIT,
                                                      LEG18_PW_HASH_ALG};

/**
 * @brief decrypts a given encrypted text with the given password and
 * cryptParameter.
 * @param ciphertext_hex the hex encoded cipher
 * @param cipher_len the lenght of the ciphertext. This is not the length of the
 * hex encoded ciphertext, but of the original plaintext + mac_len.
 * @param password the password used for encryption
 * @return a pointer to the decrypted text. It has to be freed after use. If the
 * decryption fails @c NULL is returned.
 * @note this function is only used to decrypt ciphers encrypted before version
 * 2.1.0
 */
unsigned char* crypt_decrypt_hex_withParams(char*         ciphertext_hex,
                                            unsigned long cipher_len,
                                            const char*   password,
                                            char nonce_hex[], char salt_hex[],
                                            struct cryptParameter params) {
  if (cipher_len < params.mac_len) {
    oidc_errno = OIDC_ECRYPM;
    return NULL;
  }
  logger(DEBUG, "Decrypt using hex encoding");
  unsigned char* decrypted =
      secAlloc(sizeof(unsigned char) * (cipher_len - params.mac_len + 1));
  unsigned char* key = crypt_keyDerivation_hex(password, salt_hex, 0, params);
  if (key == NULL) {
    return NULL;
  }
  unsigned char nonce[params.nonce_len];
  unsigned char ciphertext[cipher_len];
  sodium_hex2bin(nonce, params.nonce_len, nonce_hex, 2 * params.nonce_len, NULL,
                 NULL, NULL);
  sodium_hex2bin(ciphertext, cipher_len, ciphertext_hex, 2 * cipher_len, NULL,
                 NULL, NULL);
  if (crypto_secretbox_open_easy(decrypted, ciphertext, cipher_len, nonce,
                                 key) != 0) {
    secFree(key);
    logger(NOTICE, "Decryption failed.");
    secFree(decrypted);
    /* If we get here, the Message was a forgery. This means someone (or the
     * network) somehow tried to tamper with the message*/
    oidc_errno = OIDC_EPASS;
    return NULL;
  }
  secFree(key);
  return decrypted;
}

/**
 * @brief decrypts a given encrypted text with the given password.
 * @param ciphertext_hex the hex encoded ciphertext to be decrypted
 * @param cipher_len the lenght of the ciphertext. This is not the length of the
 * hex encoded ciphertext, but of the original plaintext + mac_len.
 * @param password the passwod used for encryption
 * @param nonce_hex the hex encoded nonce used for encryption
 * @param salt_hex the hex encoded salt used for encryption
 * @return a pointer to the decrypted text. It has to be freed after use. If the
 * decryption failed @c NULL is returned.
 * @note this function is only used to decrypt ciphers encrypted before version
 * 2.1.0 - for other ciphers use @c crypt_decrypt_base64
 * @deprecated only use this function for backwards compatibility
 */
unsigned char* crypt_decrypt_hex(char* ciphertext_hex, unsigned long cipher_len,
                                 const char* password, char nonce_hex[],
                                 char salt_hex[]) {
  logger(DEBUG, "Trying to decrypt hex encoded cipher using legacy18Params");
  unsigned char* res18 =
      crypt_decrypt_hex_withParams(ciphertext_hex, cipher_len, password,
                                   nonce_hex, salt_hex, legacy_18_cryptParams);
  oidc_error_t error18 = oidc_errno;
  if (res18 != NULL) {
    return res18;
  }
  logger(DEBUG, "Trying to decrypt hex encoded cipher using legacy23Params");
  unsigned char* res23 =
      crypt_decrypt_hex_withParams(ciphertext_hex, cipher_len, password,
                                   nonce_hex, salt_hex, legacy_23_cryptParams);
  oidc_error_t error23 = oidc_errno;
  if (res23 != NULL) {
    return res23;
  }
  if (error23 == error18) {
    oidc_errno = error18;
  } else {
    oidc_errno = OIDC_EPASS;  // only errors possible are OIDC_ECRPM and
                              // OIDC_EPASS; if 18 and 23 deliver different
                              // errors, EPASS is "more successful"
  }
  return NULL;
}

/**
 * @brief derivates a key from the given password
 * @param password the password to be used for key derivation
 * @param salt_hex a pointer to a 2*params.salt_len+1 big buffer. If @p
 * generateNewSalt is set, the generated salt will be stored here, otherwise the
 * stored salt will be used
 * @param generateNewSalt indicates if a new salt should be generated or if
 * @p salt_hex should be used. If you use this function for encryption
 * @p generateNewSalt should be @c 1; for decryption @c 0
 * @param params the crypt parameters to be used
 * @return a pointer to the derivated key. It has to be freed after usage.
 * @note this function is only used for keyDerivation with hex encoded salt
 * (before version 2.1.0) - see also @c crypt_keyDerivation_base64
 * @deprecated use this function only to derivate a key to compare it to one
 * derivated before version 2.1.0; it is deprecated and should not be used to
 * derivate new keys.
 */
unsigned char* crypt_keyDerivation_hex(const char* password, char salt_hex[],
                                       int                   generateNewSalt,
                                       struct cryptParameter params) {
  logger(DEBUG, "Derivate key using hex encoding");
  if (generateNewSalt == 1) {
    logger(WARNING, "%s is deprecated", __func__);
    printImportant("%s is deprecated", __func__);
  }
  unsigned char* key = secAlloc(sizeof(unsigned char) * (params.key_len + 1));
  unsigned char  salt[params.salt_len];
  if (generateNewSalt) {
    /* Choose a random salt */
    randombytes_buf(salt, params.salt_len);
    sodium_bin2hex(salt_hex, 2 * params.salt_len + 1, salt, params.salt_len);
  } else {
    sodium_hex2bin(salt, params.salt_len, salt_hex, 2 * params.salt_len, NULL,
                   NULL, NULL);
  }
  if (crypto_pwhash(key, params.key_len, password, strlen(password), salt,
                    params.hash_ops_limit, params.hash_mem_limit,
                    params.hash_alg) != 0) {
    secFree(key);
    logger(ALERT,
           "Could not derivate key. Probably because system out of memory.\n");
    oidc_errno = OIDC_EMEM;
    return NULL;
  }
  return key;
}
