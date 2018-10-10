#include "crypt.h"
#include "oidc_error.h"
#include "utils/cleaner.h"

#include <syslog.h>

/** @fn void initCrypt()
 * @brief initializes random number generator
 */
void initCrypt() { randombytes_stir(); }

/** @fn char* encrypt(const unsigned char* text, const char* password, char
 * nonce_hex[2*NONCE_LEN+1], char salt_hex[2*SALT_LEN+1])
 * @brief encrypts a given text with the given password.
 * @param text the nullterminated text
 * @param password the nullterminated password, used for encryption
 * @param nonce_hex a pointer to the location where the used nonce will be
 * stored hex encoded. The buffer should be 2*NONCE_LEN+1
 * @param salt_hex a pointer to the location where the used salt will be
 * stored hex encoded. The buffer should be 2*SALT_LEN+1
 * @return a pointer to the encrypted text. It has to be freed after use.
 */
char* crypt_encrypt(const unsigned char* text, const char* password,
                    char nonce_hex[2 * NONCE_LEN + 1],
                    char salt_hex[2 * SALT_LEN + 1]) {
  unsigned char nonce[NONCE_LEN];
  randombytes_buf(nonce, NONCE_LEN);
  sodium_bin2hex(nonce_hex, 2 * NONCE_LEN + 1, nonce, NONCE_LEN);
  unsigned char  ciphertext[MAC_LEN + strlen((char*)text)];
  unsigned char* key = crypt_keyDerivation(password, salt_hex, 1);

  crypto_secretbox_easy(ciphertext, text, strlen((char*)text), nonce, key);

  secFree(key);
  char* ciphertext_hex =
      secAlloc(sizeof(char) * (2 * (MAC_LEN + strlen((char*)text)) + 1));
  sodium_bin2hex(ciphertext_hex, 2 * (MAC_LEN + strlen((char*)text)) + 1,
                 ciphertext, MAC_LEN + strlen((char*)text));

  return ciphertext_hex;
}

/** @fn unsigned char* decrypt(char* ciphertext_hex, unsigned long cipher_len,
 * const char* password, char nonce_hex[2*NONCE_LEN+1], char
 * salt_hex[2*SALT_LEN+1])
 * @brief decrypts a given encrypted text with the given password.
 * @param ciphertext_hex the hex encoded ciphertext to be decrypted
 * @param cipher_len the lenght of the ciphertext. This is not the length of the
 * hex encoded ciphertext. It should be length of the unencrypted text + MAC_LEN
 * @param password the passwod used for encryption
 * @param nonce_hex the hex encoded nonce used for encryption
 * @param salt_hex the hex encoded salt used for encryption
 * @return a pointer to the decrypted text. It has to be freed after use. If the
 * decryption failed NULL is returned.
 */
unsigned char* crypt_decrypt(char* ciphertext_hex, unsigned long cipher_len,
                             const char* password,
                             char        nonce_hex[2 * NONCE_LEN + 1],
                             char        salt_hex[2 * SALT_LEN + 1]) {
  if (cipher_len < MAC_LEN) {
    oidc_errno = OIDC_ECRYPM;
    return NULL;
  }
  unsigned char* decrypted =
      secAlloc(sizeof(unsigned char) * (cipher_len - MAC_LEN + 1));
  unsigned char* key = crypt_keyDerivation(password, salt_hex, 0);
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
  syslog(LOG_AUTHPRIV | LOG_DEBUG, "Decrypted config is: %s\n", decrypted);
  return decrypted;
}

/** @fn unsigned char* keyDerivation(const char* password, char
 * salt_hex[2*SALT_LEN+1], int generateNewSalt)
 * @brief derivates a key from the given password
 * @param password the password use for key derivation
 * @param salt_hex a pointer to a 2*SALT_LEN+1 big buffer. If \p generateNewSalt
 * is set, the generated salt will be stored here, otherwise the stored salt
 * will be used
 * @param generateNewSalt indicates if a new salt should be generated or if
 * salt_hex should be used. If you use this function for encryption \p
 * generateNewSalt should be 1; for decryption 0
 * @return a pointer to the derivated key. It has to be freed after usage.
 */
unsigned char* crypt_keyDerivation(const char* password,
                                   char        salt_hex[2 * SALT_LEN + 1],
                                   int         generateNewSalt) {
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

void randomFillHex(char buffer[], size_t buffer_size) {
  size_t        binsize = (buffer_size - 1) / 2;
  unsigned char a[binsize];
  randombytes_buf(a, binsize);
  sodium_bin2hex(buffer, buffer_size, a, binsize);
}

char* getRandomHexString(size_t size) {
  char* r = secAlloc(sizeof(char) * (size + 1));
  randomFillHex(r, size);
  return r;
}
