#include "crypt.h"
#include "memory.h"
#include "oidc_error.h"

#include <syslog.h>

/** @fn void initCrypt()
 * @brief initializes random number generator
 */
void initCrypt() { randombytes_stir(); }

struct cryptParameter {
  int nonce_len;
  int salt_len;
  int mac_len;
  int base64_variant;
  int hash_ops_limit;
  int hash_mem_limit;
  int hash_alg;
};

struct cryptParameter newCryptParameters() {
  return (struct cryptParameter){
      NONCE_LEN,         SALT_LEN,
      MAC_LEN,           sodium_base64_VARIANT_ORIGINAL,
      PW_HASH_OPS_LIMIT, PW_HASH_MEMLIMIT,
      PW_HASH_ALG};
}

struct encryptionInfo {
  char*                 encrypted_base64;
  char*                 nonce_base64;
  char*                 salt_base64;
  char*                 hash_key_base64;
  struct cryptParameter cryptParameter;
};

void secFreeEncryptionInfo(struct encryptionInfo crypt) {
  secFree(crypt.encrypted_base64);
  secFree(crypt.nonce_base64);
  secFree(crypt.salt_base64);
  secFree(crypt.hash_key_base64);
}

/**
 * @brief encrypts a given text with the given password.
 * @param text the nullterminated text
 * @param password the nullterminated password, used for encryption
 * @param nonce_base64 a pointer to the location where the used nonce will be
 * stored base64 encoded; The buffer has to be large enough.
 * @param key_str a pointer to the location where the used key_str will be
 * stored.
 * @return a pointer to the encrypted text. It has to be freed after use.
 * @note before version 2.1.0 this function used hex encoding
 */
struct encryptionInfo _crypt_encrypt(const unsigned char* text,
                                     const char*          password) {
  syslog(LOG_AUTHPRIV | LOG_DEBUG, "Encrypt using base64 encoding");
  char nonce[NONCE_LEN];
  randombytes_buf(nonce, NONCE_LEN);
  char* salt_base64 = secAlloc(
      sodium_base64_ENCODED_LEN(SALT_LEN, sodium_base64_VARIANT_ORIGINAL) + 1);
  unsigned char  ciphertext[MAC_LEN + strlen((char*)text)];
  struct key_set keys = crypt_keyDerivation_base64(password, salt_base64, 1);
  if (keys.encryption_key == NULL) {
    secFree(salt_base64);
    return (struct encryptionInfo){NULL};
  }

  if (crypto_secretbox_easy(ciphertext, text, strlen((char*)text),
                            (unsigned char*)nonce,
                            (unsigned char*)keys.encryption_key) != 0) {
    secFree(salt_base64);
    oidc_errno = OIDC_ECRYPT;
    return (struct encryptionInfo){NULL};
  }
  secFree(keys.encryption_key);
  char* ciphertext_base64 =
      toBase64((char*)ciphertext, MAC_LEN + strlen((char*)text));
  char* hash_key_base64 = toBase64(keys.hash_key, KEY_LEN);
  char* nonce_base64    = toBase64(nonce, NONCE_LEN);
  return (struct encryptionInfo){ciphertext_base64, hash_key_base64,
                                 nonce_base64, salt_base64,
                                 newCryptParameters()};
}

char* crypt_encrypt(const char* text, const char* password) {
  struct encryptionInfo cry = _crypt_encrypt((unsigned char*)text, password);
  if (cry.encrypted_base64 == NULL) {
    return NULL;
  }
  // Current config file format:
  // 1 cipher_len
  // 2 nonce_base64
  // 3 salt_base64
  // 4 crypt parameters
  // 5 cipher_base64
  // [6 version] // Not included here
  const char* const fmt        = "%lu\n%s\n%s\n%d:%d:%d:%d:%d:%d:%d\n%s";
  size_t            cipher_len = strlen(text) + cry.cryptParameter.mac_len;
  char*             ret        = oidc_sprintf(
      fmt, cipher_len, cry.nonce_base64, cry.salt_base64,
      cry.cryptParameter.nonce_len, cry.cryptParameter.salt_len,
      cry.cryptParameter.mac_len, cry.cryptParameter.base64_variant,
      cry.cryptParameter.hash_ops_limit, cry.cryptParameter.hash_mem_limit,
      cry.cryptParameter.hash_alg, cry.encrypted_base64);
  secFreeEncryptionInfo(cry);
  return ret;
}

/**
 * @brief decrypts a given encrypted text with the given password.
 * @param ciphertext_base64 the base64 encoded ciphertext to be decrypted
 * @param cipher_len the lenght of the ciphertext. This is not the length of the
 * base64 encoded ciphertext, but of the original plaintext.
 * @param password the passwod used for encryption
 * @param nonce_base64 the base64 encoded nonce used for encryption
 * @param key_str the key_str used for encryption
 * @return a pointer to the decrypted text. It has to be freed after use. If the
 * decryption failed @c NULL is returned.
 * @note this function is only used to decrypt ciphers encrypted with version
 * 2.1.0 or higher - for ciphers encrypted before 2.1.0 use @c crypt_decrypt_hex
 */
unsigned char* crypt_decrypt_base64(struct encryptionInfo crypt,
                                    unsigned long         cipher_len,
                                    const char*           password) {
  if (crypt.encrypted_base64 == NULL || crypt.hash_key_base64 == NULL ||
      password == NULL || crypt.nonce_base64 == NULL ||
      crypt.salt_base64 == NULL) {
    oidc_setArgNullFuncError(__func__);
    return NULL;
  }
  if (cipher_len < MAC_LEN) {
    oidc_errno = OIDC_ECRYPM;
    return NULL;
  }
  syslog(LOG_AUTHPRIV | LOG_DEBUG, "Decrypt using base64 encoding");
  struct key_set keys =
      crypt_keyDerivation_base64(password, crypt.salt_base64, 0);
  char* computed_hash_key_base64 = toBase64(keys.hash_key, KEY_LEN);
  secFree(keys.hash_key);
  if (!strequal(computed_hash_key_base64, crypt.hash_key_base64)) {
    secFree(keys.encryption_key);
    secFree(computed_hash_key_base64);
    oidc_errno = OIDC_EPASS;
    return NULL;
  }
  secFree(computed_hash_key_base64);

  unsigned char nonce[NONCE_LEN];
  unsigned char ciphertext[cipher_len];
  sodium_base642bin(
      nonce, NONCE_LEN, crypt.nonce_base64,
      sodium_base64_ENCODED_LEN(NONCE_LEN, sodium_base64_VARIANT_ORIGINAL),
      NULL, NULL, NULL, sodium_base64_VARIANT_ORIGINAL);
  sodium_base642bin(
      ciphertext, cipher_len, crypt.encrypted_base64,
      sodium_base64_ENCODED_LEN(SALT_LEN, sodium_base64_VARIANT_ORIGINAL), NULL,
      NULL, NULL, sodium_base64_VARIANT_ORIGINAL);
  unsigned char* decrypted =
      secAlloc(sizeof(unsigned char) * (cipher_len - MAC_LEN + 1));
  if (crypto_secretbox_open_easy(decrypted, ciphertext, cipher_len, nonce,
                                 (unsigned char*)keys.encryption_key) != 0) {
    secFree(keys.encryption_key);
    syslog(LOG_AUTHPRIV | LOG_NOTICE, "Decryption failed.");
    secFree(decrypted);
    /* If we get here, the Message was a forgery. This means someone (or the
     * network) somehow tried to tamper with the message*/
    oidc_errno = OIDC_EPASS;
    return NULL;
  }
  secFree(keys.encryption_key);
  return decrypted;
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
 * @deprecated only use this function for backwards incompatibility
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
                    PW_HASH_OPS_LIMIT, PW_HASH_MEMLIMIT, PW_HASH_ALG) != 0) {
    secFree(key);
    syslog(LOG_AUTHPRIV | LOG_ALERT,
           "Could not derivate key. Probably because system out of memory.\n");
    oidc_errno = OIDC_EMEM;
    return NULL;
  }
  return key;
}

char* toBase64(const char* bin, size_t len) {
  size_t base64len =
      sodium_base64_ENCODED_LEN(len, sodium_base64_VARIANT_ORIGINAL);
  char* base64 = secAlloc(base64len + 1);
  sodium_bin2base64(base64, base64len, (const unsigned char*)bin, len,
                    sodium_base64_VARIANT_ORIGINAL);
  return base64;
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
// TODO
struct key_set crypt_keyDerivation_base64(
    const char* password,
    char        salt_base64[sodium_base64_ENCODED_LEN(SALT_LEN,
                                               sodium_base64_VARIANT_ORIGINAL) +
                     1],
    int         generateNewSalt) {
  syslog(LOG_AUTHPRIV | LOG_DEBUG, "Dereviate key using base64 encoding");
  char*         key = secAlloc(sizeof(unsigned char) * (2 * KEY_LEN + 1));
  unsigned char salt[SALT_LEN];
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
  if (crypto_pwhash((unsigned char*)key, 2 * KEY_LEN, password,
                    strlen(password), salt, crypto_pwhash_OPSLIMIT_INTERACTIVE,
                    crypto_pwhash_MEMLIMIT_INTERACTIVE,
                    crypto_pwhash_ALG_DEFAULT) != 0) {
    secFree(key);
    syslog(LOG_AUTHPRIV | LOG_ALERT,
           "Could not derivate key. Probably because system out of memory.\n");
    oidc_errno = OIDC_EMEM;
    return (struct key_set){NULL, NULL};
  }
  char* encryption_key = oidc_strncopy(key, KEY_LEN);
  char* hash_key       = oidc_strcopy(key + KEY_LEN);
  secFree(key);
  struct key_set keys = {encryption_key, hash_key};
  return keys;
}

void randomFillBase64UrlSafe(char buffer[], size_t buffer_size) {
  unsigned char bin[buffer_size];
  randombytes_buf(bin, buffer_size);
  sodium_bin2base64(buffer, buffer_size, bin, buffer_size,
                    sodium_base64_VARIANT_URLSAFE_NO_PADDING);
  sodium_memzero(bin, buffer_size);
}
