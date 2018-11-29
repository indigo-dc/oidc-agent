#include "crypt.h"
#include "list/list.h"
#include "memory.h"
#include "oidc_error.h"
#include "utils/listUtils.h"

#include <syslog.h>

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

/** @fn void initCrypt()
 * @brief initializes random number generator
 */
void initCrypt() { randombytes_stir(); }

struct cryptParameter {
  size_t nonce_len;
  size_t salt_len;
  size_t mac_len;
  size_t key_len;
  int    base64_variant;
  int    hash_ops_limit;
  int    hash_mem_limit;
  int    hash_alg;
};

static struct cryptParameter legacy_23_cryptParams = {LEG23_NONCE_LEN,
                                                      LEG23_SALT_LEN,
                                                      LEG23_MAC_LEN,
                                                      LEG23_KEY_LEN,
                                                      0,
                                                      LEG23_PW_HASH_OPSLIMIT,
                                                      LEG23_PW_HASH_MEMLIMIT,
                                                      LEG23_PW_HASH_ALG};
static struct cryptParameter legacy_18_cryptParams = {LEG18_NONCE_LEN,
                                                      LEG18_SALT_LEN,
                                                      LEG18_MAC_LEN,
                                                      LEG18_KEY_LEN,
                                                      0,
                                                      LEG18_PW_HASH_OPSLIMIT,
                                                      LEG18_PW_HASH_MEMLIMIT,
                                                      LEG18_PW_HASH_ALG};

struct cryptParameter newCryptParameters() {
  return (struct cryptParameter){
      SODIUM_NONCE_LEN,        SODIUM_SALT_LEN,       SODIUM_MAC_LEN,
      SODIUM_KEY_LEN,          SODIUM_BASE64_VARIANT, SODIUM_PW_HASH_OPSLIMIT,
      SODIUM_PW_HASH_MEMLIMIT, SODIUM_PW_HASH_ALG};
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
  char nonce[SODIUM_NONCE_LEN];
  randombytes_buf(nonce, SODIUM_NONCE_LEN);
  char* salt_base64 =
      secAlloc(sodium_base64_ENCODED_LEN(SODIUM_SALT_LEN,
                                         sodium_base64_VARIANT_ORIGINAL) +
               1);
  unsigned char         ciphertext[SODIUM_MAC_LEN + strlen((char*)text)];
  struct cryptParameter cryptParams = newCryptParameters();
  struct key_set        keys =
      crypt_keyDerivation_base64(password, salt_base64, 1, &cryptParams);
  if (keys.encryption_key == NULL) {
    secFree(salt_base64);
    secFree(keys.hash_key);
    return (struct encryptionInfo){NULL};
  }

  if (crypto_secretbox_easy(ciphertext, text, strlen((char*)text),
                            (unsigned char*)nonce,
                            (unsigned char*)keys.encryption_key) != 0) {
    secFree(salt_base64);
    oidc_errno = OIDC_EENCRYPT;
    return (struct encryptionInfo){NULL};
  }
  secFree(keys.encryption_key);
  char* ciphertext_base64 =
      toBase64((char*)ciphertext, SODIUM_MAC_LEN + strlen((char*)text));
  char* hash_key_base64 = toBase64(keys.hash_key, SODIUM_KEY_LEN);
  secFree(keys.hash_key);
  char* nonce_base64 = toBase64(nonce, SODIUM_NONCE_LEN);
  return (struct encryptionInfo){ciphertext_base64, nonce_base64, salt_base64,
                                 hash_key_base64, cryptParams};
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
  // 6 hash_key_base64
  // [7 version] // Not included here
  const char* const fmt = "%lu\n%s\n%s\n%lu:%lu:%lu:%lu:%d:%d:%d:%d\n%s\n%s";
  size_t            cipher_len = strlen(text) + cry.cryptParameter.mac_len;
  char*             ret        = oidc_sprintf(
      fmt, cipher_len, cry.nonce_base64, cry.salt_base64,
      cry.cryptParameter.nonce_len, cry.cryptParameter.salt_len,
      cry.cryptParameter.mac_len, cry.cryptParameter.key_len,
      cry.cryptParameter.base64_variant, cry.cryptParameter.hash_ops_limit,
      cry.cryptParameter.hash_mem_limit, cry.cryptParameter.hash_alg,
      cry.encrypted_base64, cry.hash_key_base64);
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
  if (cipher_len < crypt.cryptParameter.mac_len) {
    oidc_errno = OIDC_ECRYPM;
    return NULL;
  }
  struct key_set keys = crypt_keyDerivation_base64(password, crypt.salt_base64,
                                                   0, &(crypt.cryptParameter));
  char*          computed_hash_key_base64 =
      toBase64(keys.hash_key, crypt.cryptParameter.key_len);
  secFree(keys.hash_key);
  if (sodium_memcmp(computed_hash_key_base64, crypt.hash_key_base64,
                    strlen(crypt.hash_key_base64)) != 0) {
    secFree(keys.encryption_key);
    secFree(computed_hash_key_base64);
    oidc_errno = OIDC_EPASS;
    return NULL;
  }
  secFree(computed_hash_key_base64);

  unsigned char nonce[crypt.cryptParameter.nonce_len];
  unsigned char ciphertext[cipher_len];
  sodium_base642bin(nonce, crypt.cryptParameter.nonce_len, crypt.nonce_base64,
                    sodium_base64_ENCODED_LEN(crypt.cryptParameter.nonce_len,
                                              sodium_base64_VARIANT_ORIGINAL),
                    NULL, NULL, NULL, sodium_base64_VARIANT_ORIGINAL);
  sodium_base642bin(
      ciphertext, cipher_len, crypt.encrypted_base64,
      sodium_base64_ENCODED_LEN(cipher_len, sodium_base64_VARIANT_ORIGINAL),
      NULL, NULL, NULL, sodium_base64_VARIANT_ORIGINAL);
  unsigned char* decrypted = secAlloc(
      sizeof(unsigned char) * (cipher_len - crypt.cryptParameter.mac_len + 1));
  if (crypto_secretbox_open_easy(decrypted, ciphertext, cipher_len, nonce,
                                 (unsigned char*)keys.encryption_key) != 0) {
    secFree(keys.encryption_key);
    syslog(LOG_AUTHPRIV | LOG_NOTICE, "Decryption failed.");
    secFree(decrypted);
    /* If we get here, the Message was a forgery. This means someone (or the
     * network) somehow tried to tamper with the message*/
    oidc_errno = OIDC_EDECRYPT;
    return NULL;
  }
  secFree(keys.encryption_key);
  return decrypted;
}

char* crypt_decryptFromList(list_t* lines, const char* password) {
  if (lines == NULL || password == NULL) {
    oidc_setArgNullFuncError(__func__);
    return NULL;
  }
  syslog(LOG_AUTHPRIV | LOG_DEBUG, "Decrypt using base64 encoding");
  struct encryptionInfo crypt      = {};
  size_t                cipher_len = 0;
  sscanf(list_at(lines, 0)->val, "%lu", &cipher_len);
  crypt.nonce_base64     = oidc_strcopy(list_at(lines, 1)->val);
  crypt.salt_base64      = oidc_strcopy(list_at(lines, 2)->val);
  crypt.encrypted_base64 = oidc_strcopy(list_at(lines, 4)->val);
  crypt.hash_key_base64  = oidc_strcopy(list_at(lines, 5)->val);
  char*             tmp  = list_at(lines, 3)->val;
  const char* const fmt  = "%lu:%lu:%lu:%lu:%d:%d:%d:%d";
  sscanf(tmp, fmt, &crypt.cryptParameter.nonce_len,
         &crypt.cryptParameter.salt_len, &crypt.cryptParameter.mac_len,
         &crypt.cryptParameter.key_len, &crypt.cryptParameter.base64_variant,
         &crypt.cryptParameter.hash_ops_limit,
         &crypt.cryptParameter.hash_mem_limit, &crypt.cryptParameter.hash_alg);
  char* ret = (char*)crypt_decrypt_base64(crypt, cipher_len, password);
  secFreeEncryptionInfo(crypt);
  return ret;
}

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
    list_destroy(lines);
    return NULL;
  }
  char* ret = crypt_decryptFromList(lines, password);
  list_destroy(lines);
  return ret;
}

unsigned char* crypt_decrypt_hex_withParams(char*         ciphertext_hex,
                                            unsigned long cipher_len,
                                            const char*   password,
                                            char nonce_hex[], char salt_hex[],
                                            struct cryptParameter params) {
  if (cipher_len < params.mac_len) {
    oidc_errno = OIDC_ECRYPM;
    return NULL;
  }
  syslog(LOG_AUTHPRIV | LOG_DEBUG, "Decrypt using hex encoding");
  unsigned char* decrypted =
      secAlloc(sizeof(unsigned char) * (cipher_len - params.mac_len + 1));
  unsigned char* key = crypt_keyDerivation_hex(password, salt_hex, 0, params);
  unsigned char  nonce[params.nonce_len];
  unsigned char  ciphertext[cipher_len];
  sodium_hex2bin(nonce, params.nonce_len, nonce_hex, 2 * params.nonce_len, NULL,
                 NULL, NULL);
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
                                 const char* password, char nonce_hex[],
                                 char salt_hex[]) {
  syslog(LOG_AUTHPRIV | LOG_DEBUG,
         "Trying to decrypt hex encoded cipher using legacy18Params");
  unsigned char* res18 =
      crypt_decrypt_hex_withParams(ciphertext_hex, cipher_len, password,
                                   nonce_hex, salt_hex, legacy_18_cryptParams);
  oidc_error_t error18 = oidc_errno;
  if (res18 != NULL) {
    return res18;
  }
  syslog(LOG_AUTHPRIV | LOG_DEBUG,
         "Trying to decrypt hex encoded cipher using legacy23Params");
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
                              // errors, EPASS is "more successfull"
  }
  return NULL;
}

/**
 * @brief derivates a key from the given password
 * @param password the password use for key derivation
 * @param salt_hex a pointer to a 2*LEG_SALT_LEN+1 big buffer. If @p
 * generateNewSalt is set, the generated salt will be stored here, otherwise the
 * stored salt will be used
 * @param generateNewSalt indicates if a new salt should be generated or if
 * @p salt_hex should be used. If you use this function for encryption
 * @p generateNewSalt should be @c 1; for decryption @c 0
 * @return a pointer to the derivated key. It has to be freed after usage.
 * @note this function is only used for keyDerivation with hex encoded salt
 * (before version 2.1.0) - see also @c crypt_keyDerivation_base64
 * @deprecated use this function only to derivate a key to compare it to one
 * derivate before version 2.1.0; it is deprecated to use it for new keys.
 */
unsigned char* crypt_keyDerivation_hex(const char* password, char salt_hex[],
                                       int                   generateNewSalt,
                                       struct cryptParameter params) {
  syslog(LOG_AUTHPRIV | LOG_DEBUG, "Derivate key using hex encoding");
  if (generateNewSalt == 1) {
    syslog(LOG_AUTHPRIV | LOG_WARNING, "%s is deprecated", __func__);
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
 * @param salt_base64 a pointer to a big enough buffer. If @p
 * generateNewSalt is set, the generated salt will be stored here, otherwise the
 * stored salt will be used
 * @param generateNewSalt indicates if a new salt should be generated or if
 * @p salt_base64 should be used. If you use this function for encryption
 * @p generateNewSalt should be @c 1; for decryption @c 0
 * @return a pointer to the derivated key. It has to be freed after usage.
 * @note this function is only used to keyDerivation with base64 encoded salt
 * (since version 2.1.0) - see also @c crypt_keyDerivation_hex
 */
struct key_set crypt_keyDerivation_base64(const char* password,
                                          char        salt_base64[],
                                          int         generateNewSalt,
                                          struct cryptParameter* cryptParams) {
  syslog(LOG_AUTHPRIV | LOG_DEBUG, "Derivate key using base64 encoding");
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
    sodium_base642bin(salt, cryptParams->salt_len, salt_base64,
                      sodium_base64_ENCODED_LEN(cryptParams->salt_len,
                                                sodium_base64_VARIANT_ORIGINAL),
                      NULL, NULL, NULL, sodium_base64_VARIANT_ORIGINAL);
  }
  if (crypto_pwhash((unsigned char*)key, 2 * cryptParams->key_len, password,
                    strlen(password), salt, crypto_pwhash_OPSLIMIT_INTERACTIVE,
                    crypto_pwhash_MEMLIMIT_INTERACTIVE,
                    crypto_pwhash_ALG_DEFAULT) != 0) {
    secFree(key);
    syslog(LOG_AUTHPRIV | LOG_ALERT,
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

void randomFillBase64UrlSafe(char buffer[], size_t buffer_size) {
  unsigned char bin[buffer_size];
  randombytes_buf(bin, buffer_size);
  sodium_bin2base64(buffer, buffer_size, bin, buffer_size,
                    sodium_base64_VARIANT_URLSAFE_NO_PADDING);
  sodium_memzero(bin, buffer_size);
}
