#include "crypt.h"

#include <sodium.h>
#include <string.h>

#include "utils/listUtils.h"
#include "utils/logger.h"
#include "utils/memory.h"
#include "utils/oidc_error.h"
#include "utils/string/stringUtils.h"

// use these for new encryptions
#define SODIUM_KEY_LEN crypto_secretbox_KEYBYTES
#define SODIUM_SALT_LEN crypto_pwhash_SALTBYTES
#define SODIUM_NONCE_LEN crypto_secretbox_NONCEBYTES
#define SODIUM_MAC_LEN crypto_secretbox_MACBYTES
#define SODIUM_BASE64_VARIANT sodium_base64_VARIANT_ORIGINAL
#define SODIUM_PW_HASH_ALG crypto_pwhash_ALG_DEFAULT
#define SODIUM_PW_HASH_OPSLIMIT crypto_pwhash_OPSLIMIT_INTERACTIVE
#define SODIUM_PW_HASH_MEMLIMIT crypto_pwhash_MEMLIMIT_INTERACTIVE

/**
 * @brief initializes random number generator
 */
void initCrypt() { randombytes_stir(); }

/**
 * @brief returns current cryptParameters
 * @return a cryptParameter struct
 */
struct cryptParameter newCryptParameters() {
  return (struct cryptParameter){
      SODIUM_NONCE_LEN,        SODIUM_SALT_LEN,       SODIUM_MAC_LEN,
      SODIUM_KEY_LEN,          SODIUM_BASE64_VARIANT, SODIUM_PW_HASH_OPSLIMIT,
      SODIUM_PW_HASH_MEMLIMIT, SODIUM_PW_HASH_ALG};
}

/**
 * @brief encrypts a given text with the given password.
 * @param text the nullterminated text
 * @param password the nullterminated password, used for encryption
 * @return a pointer to an encryptionInfo struct; Has to be freed after usage.
 * usage using @c secFreeEncryptionInfo
 */
struct encryptionInfo* _crypt_encrypt(const unsigned char* text,
                                      const char*          password) {
  if (text == NULL || password == NULL) {
    oidc_setArgNullFuncError(__func__);
    return NULL;
  }
  logger(DEBUG, "Encrypt using base64 encoding");
  char* salt_base64 =
      secAlloc(sodium_base64_ENCODED_LEN(SODIUM_SALT_LEN,
                                         sodium_base64_VARIANT_ORIGINAL) +
               1);
  struct cryptParameter cryptParams = newCryptParameters();
  struct key_set        keys =
      crypt_keyDerivation_base64(password, salt_base64, 1, &cryptParams);
  if (keys.encryption_key == NULL) {
    secFree(salt_base64);
    secFree(keys.hash_key);
    return NULL;
  }

  struct encryptionInfo* result =
      crypt_encryptWithKey(text, (unsigned char*)keys.encryption_key);
  secFree(keys.encryption_key);

  char* hash_key_base64 = toBase64(keys.hash_key, SODIUM_KEY_LEN);
  secFree(keys.hash_key);
  result->salt_base64     = salt_base64;
  result->hash_key_base64 = hash_key_base64;
  result->cryptParameter  = cryptParams;
  if (result->encrypted_base64 == NULL) {
    secFreeEncryptionInfo(result);
    return NULL;
  }
  return result;
}

/**
 * @brief encrypts a given text with the given key.
 * @param text the nullterminated text
 * @param key the key to be used for encryption
 * @return a pointer to an encryptionInfo struct; Has to be freed after
 * usage using @c secFreeEncryptionInfo
 */
struct encryptionInfo* crypt_encryptWithKey(const unsigned char* text,
                                            const unsigned char* key) {
  struct cryptParameter cryptParams = newCryptParameters();
  char                  nonce[cryptParams.nonce_len];
  randombytes_buf(nonce, cryptParams.nonce_len);
  unsigned char ciphertext[cryptParams.mac_len + strlen((char*)text)];
  if (crypto_secretbox_easy(ciphertext, text, strlen((char*)text),
                            (unsigned char*)nonce, key) != 0) {
    oidc_errno = OIDC_EENCRYPT;
    return NULL;
  }
  char* ciphertext_base64 =
      toBase64((char*)ciphertext, cryptParams.mac_len + strlen((char*)text));
  char*                  nonce_base64 = toBase64(nonce, cryptParams.nonce_len);
  struct encryptionInfo* crypt        = secAlloc(sizeof(struct encryptionInfo));
  crypt->encrypted_base64             = ciphertext_base64;
  crypt->nonce_base64                 = nonce_base64;
  crypt->cryptParameter               = cryptParams;
  return crypt;
}

/**
 * @brief encrypts a given text with the given password.
 * This function uses base64 encoding
 * @param text the nullterminated text
 * @param password the nullterminated password, used for encryption
 * @return a string containing all relevant encryption information; this string
 * can be passed to @c crypt_decrypt for decryption
 * @note before version 2.1.0 this function used hex encoding
 */
char* crypt_encrypt(const char* text, const char* password) {
  struct encryptionInfo* cry = _crypt_encrypt((unsigned char*)text, password);
  if (cry == NULL || cry->encrypted_base64 == NULL) {
    return NULL;
  }
  // Current config file format:
  // 1 cipher_len
  // 2 nonce_base64
  // 3 salt_base64
  // 4 crypt parameters
  // 5 cipher_base64
  // 6 hash_key_base64
  // [7 version] // Not included here
  const char* const fmt = "%lu\n%s\n%s\n%lu:%lu:%lu:%lu:%d:%d:%d:%d\n%s\n%s";
  size_t            cipher_len = strlen(text) + cry->cryptParameter.mac_len;
  char*             ret        = oidc_sprintf(
                         fmt, cipher_len, cry->nonce_base64, cry->salt_base64,
                         cry->cryptParameter.nonce_len, cry->cryptParameter.salt_len,
                         cry->cryptParameter.mac_len, cry->cryptParameter.key_len,
                         cry->cryptParameter.base64_variant, cry->cryptParameter.hash_ops_limit,
                         cry->cryptParameter.hash_mem_limit, cry->cryptParameter.hash_alg,
                         cry->encrypted_base64, cry->hash_key_base64);
  secFreeEncryptionInfo(cry);
  return ret;
}

/**
 * @brief decrypts a given encrypted text with the given password.
 * @param crypt a encryptionInfo struct containing all relevant encryption
 * information
 * @param cipher_len the lenght of the ciphertext. This is not the length of the
 * base64 encoded ciphertext, but of the original plaintext + mac_len.
 * @param password the passwod used for encryption
 * @return a pointer to the decrypted text. It has to be freed after use. If the
 * decryption fails @c NULL is returned.
 * @note this function is only used to decrypt ciphers encrypted with version
 * 2.1.0 or higher - for ciphers encrypted before 2.1.0 use @c crypt_decrypt_hex
 */
unsigned char* crypt_decrypt_base64(struct encryptionInfo* crypt,
                                    unsigned long          cipher_len,
                                    const char*            password) {
  if (crypt == NULL || crypt->encrypted_base64 == NULL ||
      crypt->hash_key_base64 == NULL || password == NULL ||
      crypt->nonce_base64 == NULL || crypt->salt_base64 == NULL) {
    oidc_setArgNullFuncError(__func__);
    return NULL;
  }
  if (cipher_len < crypt->cryptParameter.mac_len) {
    oidc_errno = OIDC_ECRYPM;
    return NULL;
  }
  struct key_set keys = crypt_keyDerivation_base64(password, crypt->salt_base64,
                                                   0, &(crypt->cryptParameter));
  if (keys.encryption_key == NULL) {
    secFree(keys.hash_key);
    return NULL;
  }

  char* computed_hash_key_base64 =
      toBase64(keys.hash_key, crypt->cryptParameter.key_len);
  secFree(keys.hash_key);
  if (sodium_memcmp(computed_hash_key_base64, crypt->hash_key_base64,
                    strlen(crypt->hash_key_base64)) != 0) {
    secFree(keys.encryption_key);
    secFree(computed_hash_key_base64);
    oidc_errno = OIDC_EPASS;
    return NULL;
  }
  secFree(computed_hash_key_base64);
  unsigned char* decrypted = crypt_decryptWithKey(
      crypt, cipher_len, (unsigned char*)keys.encryption_key);
  secFree(keys.encryption_key);
  return decrypted;
}

/**
 * @brief decrypts a given encrypted text with the given key.
 * @param crypt a encryptionInfo struct containing all relevant encryption
 * information
 * @param cipher_len the lenght of the ciphertext. This is not the length of the
 * base64 encoded ciphertext, but of the original plaintext + mac_len.
 * @param key the key used for encryption
 * @return a pointer to the decrypted text. It has to be freed after use. If the
 * decryption fails @c NULL is returned.
 * @note this function is only used to decrypt ciphers encrypted with version
 * 2.1.0 or higher - for ciphers encrypted before 2.1.0 use @c crypt_decrypt_hex
 */
unsigned char* crypt_decryptWithKey(const struct encryptionInfo* crypt,
                                    unsigned long                cipher_len,
                                    const unsigned char*         key) {
  unsigned char nonce[crypt->cryptParameter.nonce_len];
  unsigned char ciphertext[cipher_len];
  fromBase64(crypt->nonce_base64, crypt->cryptParameter.nonce_len, nonce);
  fromBase64(crypt->encrypted_base64, cipher_len, ciphertext);
  unsigned char* decrypted = secAlloc(
      sizeof(unsigned char) * (cipher_len - crypt->cryptParameter.mac_len + 1));
  if (crypto_secretbox_open_easy(decrypted, ciphertext, cipher_len, nonce,
                                 key) != 0) {
    logger(NOTICE, "Decryption failed.");
    secFree(decrypted);
    /* If we get here, the Message was a forgery. This means someone (or the
     * network) somehow tried to tamper with the message*/
    oidc_errno = OIDC_EDECRYPT;
    return NULL;
  }
  return decrypted;
}

/**
 * @brief decrypts a given encrypted text with the given password.
 * @param lines a list of strings containing all relevant encryption
 * information
 * @param password the password used for encryption
 * @return a pointer to the decrypted text. It has to be freed after use. If the
 * decryption fails @c NULL is returned.
 * @note this function is only used to decrypt ciphers encrypted with version
 * 2.1.0 or higher - for ciphers encrypted before 2.1.0 use @c crypt_decrypt_hex
 */
char* crypt_decryptFromList(list_t* lines, const char* password) {
  if (lines == NULL || password == NULL) {
    oidc_setArgNullFuncError(__func__);
    return NULL;
  }
  logger(DEBUG, "Decrypt using base64 encoding");
  struct encryptionInfo* crypt      = secAlloc(sizeof(struct encryptionInfo));
  size_t                 cipher_len = 0;
  sscanf(list_at(lines, 0)->val, "%lu", &cipher_len);
  crypt->nonce_base64     = oidc_strcopy(list_at(lines, 1)->val);
  crypt->salt_base64      = oidc_strcopy(list_at(lines, 2)->val);
  crypt->encrypted_base64 = oidc_strcopy(list_at(lines, 4)->val);
  crypt->hash_key_base64  = oidc_strcopy(list_at(lines, 5)->val);
  char*             tmp   = list_at(lines, 3)->val;
  const char* const fmt   = "%lu:%lu:%lu:%lu:%d:%d:%d:%d";
  sscanf(tmp, fmt, &crypt->cryptParameter.nonce_len,
         &crypt->cryptParameter.salt_len, &crypt->cryptParameter.mac_len,
         &crypt->cryptParameter.key_len, &crypt->cryptParameter.base64_variant,
         &crypt->cryptParameter.hash_ops_limit,
         &crypt->cryptParameter.hash_mem_limit,
         &crypt->cryptParameter.hash_alg);
  char* ret = (char*)crypt_decrypt_base64(crypt, cipher_len, password);
  secFreeEncryptionInfo(crypt);
  return ret;
}

/**
 * @brief decrypts a given encrypted text with the given password.
 * @param crypt_str a string containing all relevant encryption information; has
 * to be in the format as returned from @c crypt_encrypt
 * @param password the password used for encryption
 * @return a pointer to the decrypted text. It has to be freed after use. If the
 * decryption fails @c NULL is returned.
 * @note this function is only used to decrypt ciphers encrypted with version
 * 2.1.0 or higher - for ciphers encrypted before 2.1.0 use @c crypt_decrypt_hex
 */
char* crypt_decrypt(const char* crypt_str, const char* password) {
  if (crypt_str == NULL || password == NULL) {
    oidc_setArgNullFuncError(__func__);
    return NULL;
  }
  list_t* lines = delimitedStringToList(crypt_str, '\n');
  if (lines == NULL) {
    return NULL;
  }
  if (lines->len < 5) {
    oidc_errno = OIDC_ECRYPM;
    secFreeList(lines);
    return NULL;
  }
  char* ret = crypt_decryptFromList(lines, password);
  secFreeList(lines);
  return ret;
}

/**
 * @brief base64 encodes len bytes of bin
 * @param bin the binary string that should be encoded
 * @param len the number of bytes that should be encoded
 * @param variant the base64 variant to be used
 * @return a pointer to string holding the base64 encoded binary; has to be
 * freed after usage.
 */
char* toBase64WithVariant(const char* bin, size_t len, int variant) {
  if (bin == NULL) {
    oidc_setArgNullFuncError(__func__);
    return NULL;
  }
  size_t base64len = sodium_base64_ENCODED_LEN(len, variant);
  char*  base64    = secAlloc(base64len + 1);
  if (base64 == NULL) {
    oidc_errno = OIDC_EALLOC;
    return NULL;
  }
  sodium_bin2base64(base64, base64len, (const unsigned char*)bin, len, variant);
  return base64;
}

/**
 * @brief base64 encodes len bytes of bin
 * @param bin the binary string that should be encoded
 * @param len the number of bytes that should be encoded
 * @return a pointer to string holding the base64 encoded binary; has to be
 * freed after usage.
 * @note base64 encoding is not url-safe
 */
char* toBase64(const char* bin, size_t len) {
  if (bin == NULL) {
    oidc_setArgNullFuncError(__func__);
    return NULL;
  }
  return toBase64WithVariant(bin, len, sodium_base64_VARIANT_ORIGINAL);
}

/**
 * @brief base64 encodes len bytes of bin
 * @param bin the binary string that should be encoded
 * @param len the number of bytes that should be encoded
 * @return a pointer to string holding the base64 encoded binary; has to be
 * freed after usage.
 * @note base64 encoding is url-safe
 */
char* toBase64UrlSafe(const char* bin, size_t len) {
  if (bin == NULL) {
    oidc_setArgNullFuncError(__func__);
    return NULL;
  }
  return toBase64WithVariant(bin, len,
                             sodium_base64_VARIANT_URLSAFE_NO_PADDING);
}

/**
 * @brief decodes a base64 encoded string an places it in bin
 * @param base64 the nullterminated base64 encoded string
 * @param bin_len the length of the buffer @p bin
 * @param bin the buffer where the decoded string should be placed
 * @return @c 0 on success, @c -1 otherwise
 */
int fromBase64(const char* base64, size_t bin_len, unsigned char* bin) {
  if (base64 == NULL || bin == NULL) {
    oidc_setArgNullFuncError(__func__);
    return oidc_errno;
  }
  return sodium_base642bin(
      bin, bin_len, base64,
      sodium_base64_ENCODED_LEN(bin_len, sodium_base64_VARIANT_ORIGINAL), NULL,
      NULL, NULL, sodium_base64_VARIANT_ORIGINAL);
}

/**
 * @brief decodes a base64 encoded string an places it in bin
 * @param base64 the nullterminated base64 encoded string
 * @param bin_len the length of the buffer @p bin
 * @param bin the buffer where the decoded string should be placed
 * @return @c 0 on success, @c -1 otherwise
 */
int fromBase64UrlSafe(const char* base64, size_t bin_len, unsigned char* bin) {
  if (base64 == NULL || bin == NULL) {
    oidc_setArgNullFuncError(__func__);
    return oidc_errno;
  }
  return sodium_base642bin(
      bin, bin_len, base64,
      sodium_base64_ENCODED_LEN(bin_len,
                                sodium_base64_VARIANT_URLSAFE_NO_PADDING),
      NULL, NULL, NULL, sodium_base64_VARIANT_URLSAFE_NO_PADDING);
}

/**
 * @brief hashes a string using SHA256
 * @param str the nullterminated string that should be hashed
 * @return a pointer to the sha256 hash; not zeroterminated; has to be
 * freed after usage.
 */
char* sha256(const char* str) {
  if (str == NULL) {
    oidc_setArgNullFuncError(__func__);
    return NULL;
  }
  char* sha = secAlloc(crypto_hash_sha256_BYTES);
  if (sha == NULL) {
    oidc_errno = OIDC_EALLOC;
    return NULL;
  }
  crypto_hash_sha256((unsigned char*)sha, (unsigned char*)str, strlen(str));
  return sha;
}

/**
 * @brief hashes a string using SHA256 and encodes it with base64
 * @param str the nullterminated string that should be hashed
 * @return a pointer to the s256 hash; has to be
 * freed after usage.
 */
char* s256(const char* str) {
  if (str == NULL) {
    oidc_setArgNullFuncError(__func__);
    return NULL;
  }
  char* sha = sha256(str);
  if (sha == NULL) {
    return NULL;
  }
  char* s = toBase64UrlSafe(sha, crypto_hash_sha256_BYTES);
  secFree(sha);
  return s;
}

/**
 * @brief derivates two keys from the given password
 * @param password the password to be used for key derivation
 * @param salt_base64 a pointer to a big enough buffer. If @p
 * generateNewSalt is set, the generated salt will be stored here, otherwise the
 * stored salt will be used
 * @param generateNewSalt indicates if a new salt should be generated or if
 * @p salt_base64 should be used. If you use this function for encryption
 * @p generateNewSalt should be @c 1; for decryption @c 0
 * @param cryptParameters a pointer to a cryptParameter struct that holds the
 * parameters to be used
 * @return a struct holding two pointers to the derivated keys. They have to be
 * freed after usage.
 * @note this function is only used to keyDerivation with base64 encoded salt
 * (since version 2.1.0) - see also @c crypt_keyDerivation_hex
 */
struct key_set crypt_keyDerivation_base64(const char* password,
                                          char        salt_base64[],
                                          int         generateNewSalt,
                                          struct cryptParameter* cryptParams) {
  logger(DEBUG, "Derivate key using base64 encoding");
  char* key = secAlloc(sizeof(unsigned char) * (2 * cryptParams->key_len + 1));
  unsigned char salt[cryptParams->key_len];
  if (generateNewSalt) {
    /* Choose a random salt */
    randombytes_buf(salt, cryptParams->salt_len);
    sodium_bin2base64(
        salt_base64,
        sodium_base64_ENCODED_LEN(cryptParams->salt_len,
                                  sodium_base64_VARIANT_ORIGINAL) +
            1,
        salt, cryptParams->salt_len, sodium_base64_VARIANT_ORIGINAL);
  } else {
    fromBase64(salt_base64, cryptParams->salt_len, salt);
  }
  if (crypto_pwhash((unsigned char*)key, 2 * cryptParams->key_len, password,
                    strlen(password), salt, crypto_pwhash_OPSLIMIT_INTERACTIVE,
                    crypto_pwhash_MEMLIMIT_INTERACTIVE,
                    crypto_pwhash_ALG_DEFAULT) != 0) {
    secFree(key);
    logger(ALERT,
           "Could not derivate key. Probably because system out of memory.\n");
    oidc_errno = OIDC_EMEM;
    return (struct key_set){NULL, NULL};
  }
  char* encryption_key = oidc_memcopy(key, cryptParams->key_len);
  char* hash_key =
      oidc_memcopy(key + cryptParams->key_len, cryptParams->key_len);
  secFree(key);
  struct key_set keys = {encryption_key, hash_key};
  return keys;
}

/**
 * @brief fills a buffer with random base64 characters
 * this is done by filling a buffer with random (binary) bytes and encoding this
 * buffer with an url safe base64 variant
 * @param buffer the buffer that should be filled
 * @param buffer_size the size of @p buffer
 * @note the filled buffer is url safe
 */
void randomFillBase64UrlSafe(char buffer[], size_t buffer_size) {
  unsigned char bin[buffer_size];
  randombytes_buf(bin, buffer_size);
  char base64[sodium_base64_ENCODED_LEN(
      buffer_size, sodium_base64_VARIANT_URLSAFE_NO_PADDING)];
  sodium_bin2base64(base64,
                    sodium_base64_ENCODED_LEN(
                        buffer_size, sodium_base64_VARIANT_URLSAFE_NO_PADDING),
                    bin, buffer_size, sodium_base64_VARIANT_URLSAFE_NO_PADDING);
  strncpy(buffer, base64, buffer_size);
  sodium_memzero(base64,
                 sodium_base64_ENCODED_LEN(
                     buffer_size, sodium_base64_VARIANT_URLSAFE_NO_PADDING));
  sodium_memzero(bin, buffer_size);
}
