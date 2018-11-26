#include "crypt.h"
#include "memory.h"
#include "oidc_error.h"

#include <syslog.h>

/** @fn void initCrypt()
 * @brief initializes random number generator
 */
void initCrypt() { randombytes_stir(); }

/**
 * @brief encrypts a given text with the given password.
 * @param text the nullterminated text
 * @param password the nullterminated password, used for encryption
 * @param nonce_hex a pointer to the location where the used nonce will be
 * stored hex encoded. The buffer should be 2*NONCE_LEN+1
 * @param salt_hex a pointer to the location where the used salt will be
 * stored hex encoded. The buffer should be 2*SALT_LEN+1
 * @return a pointer to the encrypted text. It has to be freed after use.
 * @note before version 2.1.0 this function used hex encoding
 */
char* crypt_encrypt(
    const unsigned char* text, const char* password,
    char nonce_base64[sodium_base64_ENCODED_LEN(
                          NONCE_LEN, sodium_base64_VARIANT_ORIGINAL) +
                      1],
    char salt_base64[sodium_base64_ENCODED_LEN(SALT_LEN,
                                               sodium_base64_VARIANT_ORIGINAL) +
                     1]) {
  syslog(LOG_AUTHPRIV | LOG_DEBUG, "Encrypt using base64 encoding");
  unsigned char nonce[NONCE_LEN];
  randombytes_buf(nonce, NONCE_LEN);
  sodium_bin2base64(
      nonce_base64,
      sodium_base64_ENCODED_LEN(NONCE_LEN, sodium_base64_VARIANT_ORIGINAL) + 1,
      nonce, NONCE_LEN, sodium_base64_VARIANT_ORIGINAL);
  unsigned char  ciphertext[MAC_LEN + strlen((char*)text)];
  unsigned char* key = crypt_keyDerivation_base64(password, salt_base64, 1);

  crypto_secretbox_easy(ciphertext, text, strlen((char*)text), nonce, key);

  secFree(key);
  char* ciphertext_hex =
      secAlloc(sizeof(char) * (2 * (MAC_LEN + strlen((char*)text)) + 1));
  sodium_bin2hex(ciphertext_hex, 2 * (MAC_LEN + strlen((char*)text)) + 1,
                 ciphertext, MAC_LEN + strlen((char*)text));

  return ciphertext_hex;
}

/**
 * @brief decrypts a given encrypted text with the given password.
 * @param ciphertext_hex the hex encoded ciphertext to be decrypted
 * @param cipher_len the lenght of the ciphertext. This is not the length of the
 * hex encoded ciphertext, but of the original plaintext.
 * @param password the passwod used for encryption
 * @param nonce_hex the hex encoded nonce used for encryption
 * @param salt_hex the hex encoded salt used for encryption
 * @return a pointer to the decrypted text. It has to be freed after use. If the
 * decryption failed @c NULL is returned.
 * @note this function is only used to decrypt ciphers encrypted before version
 * 2.1.0 - for other ciphers use @c crypt_decrypt_base64
 */
unsigned char* crypt_decrypt_hex(char* ciphertext_hex, unsigned long cipher_len,
                                 const char* password,
                                 char        nonce_hex[2 * NONCE_LEN + 1],
                                 char        salt_hex[2 * SALT_LEN + 1]) {
  if (cipher_len < MAC_LEN) {
    oidc_errno = OIDC_ECRYPM;
    return NULL;
  }
  syslog(LOG_AUTHPRIV | LOG_DEBUG, "Decrypt using hex encoding");
  syslog(LOG_AUTHPRIV | LOG_DEBUG, "%s", ciphertext_hex);
  unsigned char* decrypted =
      secAlloc(sizeof(unsigned char) * (cipher_len - MAC_LEN + 1));
  unsigned char* key = crypt_keyDerivation_hex(password, salt_hex, 0);
  unsigned char  nonce[NONCE_LEN];
  unsigned char  ciphertext[cipher_len];
  sodium_hex2bin(nonce, NONCE_LEN, nonce_hex, 2 * NONCE_LEN, NULL, NULL, NULL);
  sodium_hex2bin(ciphertext, cipher_len, ciphertext_hex, 2 * cipher_len, NULL,
                 NULL, NULL);
  if (crypto_secretbox_open_easy(decrypted, ciphertext, cipher_len, nonce,
                                 key) != 0) {
    secFree(key);
    syslog(LOG_AUTHPRIV | LOG_NOTICE, "Decryption failed.");
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
 * @param ciphertext_base64 the base64 encoded ciphertext to be decrypted
 * @param cipher_len the lenght of the ciphertext. This is not the length of the
 * base64 encoded ciphertext, but of the original plaintext.
 * @param password the passwod used for encryption
 * @param nonce_base64 the base64 encoded nonce used for encryption
 * @param salt_base64 the base64 encoded salt used for encryption
 * @return a pointer to the decrypted text. It has to be freed after use. If the
 * decryption failed @c NULL is returned.
 * @note this function is only used to decrypt ciphers encrypted with version
 * 2.1.0 or higher - for ciphers encrypted before 2.1.0 use @c crypt_decrypt_hex
 */
unsigned char* crypt_decrypt_base64(
    char* ciphertext_base64, unsigned long cipher_len, const char* password,
    char nonce_base64[sodium_base64_ENCODED_LEN(
                          NONCE_LEN, sodium_base64_VARIANT_ORIGINAL) +
                      1],
    char salt_base64[sodium_base64_ENCODED_LEN(SALT_LEN,
                                               sodium_base64_VARIANT_ORIGINAL) +
                     1]) {
  if (cipher_len < MAC_LEN) {
    oidc_errno = OIDC_ECRYPM;
    return NULL;
  }
  syslog(LOG_AUTHPRIV | LOG_DEBUG, "Decrypt using base64 encoding");
  unsigned char* decrypted =
      secAlloc(sizeof(unsigned char) * (cipher_len - MAC_LEN + 1));
  unsigned char* key = crypt_keyDerivation_base64(password, salt_base64, 0);
  unsigned char  nonce[NONCE_LEN];
  unsigned char  ciphertext[cipher_len];
  sodium_base642bin(
      nonce, NONCE_LEN, nonce_base64,
      sodium_base64_ENCODED_LEN(NONCE_LEN, sodium_base64_VARIANT_ORIGINAL),
      NULL, NULL, NULL, sodium_base64_VARIANT_ORIGINAL);
  sodium_base642bin(
      ciphertext, cipher_len, ciphertext_base64,
      sodium_base64_ENCODED_LEN(SALT_LEN, sodium_base64_VARIANT_ORIGINAL), NULL,
      NULL, NULL, sodium_base64_VARIANT_ORIGINAL);
  if (crypto_secretbox_open_easy(decrypted, ciphertext, cipher_len, nonce,
                                 key) != 0) {
    secFree(key);
    syslog(LOG_AUTHPRIV | LOG_NOTICE, "Decryption failed.");
    secFree(decrypted);
    /* If we get here, the Message was a forgery. This means someone (or the
     * network) somehow tried to tamper with the message*/
    oidc_errno = OIDC_EPASS;
    return NULL;
  }
  secFree(key);
  return decrypted;
}

// This can be pretty complex to determine where to use hex and where base64
// Maybe we should just use different funtions, or callbacks or both
// We could use base64 only for in memory stuff, on the other hand, for the file
// encryption it's not too complicated to determine base64 or hex
// To check this absolutly reliable, we could include a tag in base64 files, if
// we do this we also might want to include version information in the file ->
// but that's more of a file stuff not encryption, at least the last part
//
/**
 * @brief derivates a key from the given password
 * @param password the password use for key derivation
 * @param salt_hex a pointer to a 2*SALT_LEN+1 big buffer. If @p generateNewSalt
 * is set, the generated salt will be stored here, otherwise the stored salt
 * will be used
 * @param generateNewSalt indicates if a new salt should be generated or if
 * @p salt_hex should be used. If you use this function for encryption
 * @p generateNewSalt should be @c 1; for decryption @c 0
 * @return a pointer to the derivated key. It has to be freed after usage.
 * @note this function is only used to keyDerivation with hex encoded salt
 * (before version 2.1.0) - see also @c crypt_keyDerivation_base64
 */
unsigned char* crypt_keyDerivation_hex(const char* password,
                                       char        salt_hex[2 * SALT_LEN + 1],
                                       int         generateNewSalt) {
  syslog(LOG_AUTHPRIV | LOG_DEBUG, "Dereviate key using hex encoding");
  unsigned char* key = secAlloc(sizeof(unsigned char) * (KEY_LEN + 1));
  unsigned char  salt[SALT_LEN];
  if (generateNewSalt) {
    /* Choose a random salt */
    randombytes_buf(salt, SALT_LEN);
    sodium_bin2hex(salt_hex, 2 * SALT_LEN + 1, salt, SALT_LEN);
  } else {
    sodium_hex2bin(salt, SALT_LEN, salt_hex, 2 * SALT_LEN, NULL, NULL, NULL);
  }
  if (crypto_pwhash(key, KEY_LEN, password, strlen(password), salt,
                    crypto_pwhash_OPSLIMIT_INTERACTIVE,
                    crypto_pwhash_MEMLIMIT_INTERACTIVE,
                    crypto_pwhash_ALG_DEFAULT) != 0) {
    syslog(LOG_AUTHPRIV | LOG_ALERT,
           "Could not derivate key. Probably because system out of memory.\n");
    oidc_errno = OIDC_EMEM;
    return NULL;
  }
  return key;
}

/**
 * @brief derivates a key from the given password
 * @param password the password use for key derivation
 * @param salt_base64 a pointer big enough buffer. If @p
 * generateNewSalt is set, the generated salt will be stored here, otherwise the
 * stored salt will be used
 * @param generateNewSalt indicates if a new salt should be generated or if
 * @p salt_base64 should be used. If you use this function for encryption
 * @p generateNewSalt should be @c 1; for decryption @c 0
 * @return a pointer to the derivated key. It has to be freed after usage.
 * @note this function is only used to keyDerivation with base64 encoded salt
 * (since version 2.1.0) - see also @c crypt_keyDerivation_hex
 */
unsigned char* crypt_keyDerivation_base64(
    const char* password,
    char        salt_base64[sodium_base64_ENCODED_LEN(SALT_LEN,
                                               sodium_base64_VARIANT_ORIGINAL) +
                     1],
    int         generateNewSalt) {
  syslog(LOG_AUTHPRIV | LOG_DEBUG, "Dereviate key using base64 encoding");
  unsigned char* key = secAlloc(sizeof(unsigned char) * (KEY_LEN + 1));
  unsigned char  salt[SALT_LEN];
  if (generateNewSalt) {
    /* Choose a random salt */
    randombytes_buf(salt, SALT_LEN);
    sodium_bin2base64(
        salt_base64,
        sodium_base64_ENCODED_LEN(SALT_LEN, sodium_base64_VARIANT_ORIGINAL) + 1,
        salt, SALT_LEN, sodium_base64_VARIANT_ORIGINAL);
  } else {
    sodium_base642bin(
        salt, SALT_LEN, salt_base64,
        sodium_base64_ENCODED_LEN(SALT_LEN, sodium_base64_VARIANT_ORIGINAL),
        NULL, NULL, NULL, sodium_base64_VARIANT_ORIGINAL);
  }
  if (crypto_pwhash(key, KEY_LEN, password, strlen(password), salt,
                    crypto_pwhash_OPSLIMIT_INTERACTIVE,
                    crypto_pwhash_MEMLIMIT_INTERACTIVE,
                    crypto_pwhash_ALG_DEFAULT) != 0) {
    syslog(LOG_AUTHPRIV | LOG_ALERT,
           "Could not derivate key. Probably because system out of memory.\n");
    oidc_errno = OIDC_EMEM;
    return NULL;
  }
  return key;
}

void randomFillBase64UrlSafe(char buffer[], size_t buffer_size) {
  unsigned char bin[buffer_size];
  randombytes_buf(bin, buffer_size);
  sodium_bin2base64(buffer, buffer_size, bin, buffer_size,
                    sodium_base64_VARIANT_URLSAFE_NO_PADDING);
  sodium_memzero(bin, buffer_size);
}
