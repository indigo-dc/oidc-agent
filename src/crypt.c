#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <sodium.h>

#include "crypt.h"
#include "config.h"
#include "file_io.h"


char* encrypt(const unsigned char* text, const char* password, char nonce_hex[2*NONCE_LEN+1], char salt_hex[2*SALT_LEN+1]) {
  unsigned char nonce[NONCE_LEN];
  randombytes_buf(nonce, NONCE_LEN);
  sodium_bin2hex(nonce_hex, 2*NONCE_LEN+1, nonce, NONCE_LEN);
  unsigned char ciphertext[MAC_LEN + strlen((char*)text)];
  unsigned char* key = keyDerivation(password, salt_hex, 1);
  crypto_secretbox_easy(ciphertext, text, strlen((char*)text), nonce, key);
  memset(key, 0, KEY_LEN);
  free(key);
  char* ciphertext_hex = calloc(sizeof(char), 2*(MAC_LEN + strlen((char*)text))+1);
  sodium_bin2hex(ciphertext_hex, 2*(MAC_LEN + strlen((char*)text))+1, ciphertext, MAC_LEN + strlen((char*)text));
  return ciphertext_hex;
}

unsigned char* decrypt(char* ciphertext_hex, unsigned long cipher_len, const char* password, char nonce_hex[2*NONCE_LEN+1], char salt_hex[2*SALT_LEN+1]) {
  unsigned char* decrypted = calloc(sizeof(unsigned char), cipher_len-MAC_LEN+1);
  unsigned char* key = keyDerivation(password, salt_hex, 0);
  unsigned char nonce[NONCE_LEN];
  unsigned char ciphertext[cipher_len];
  sodium_hex2bin(nonce, NONCE_LEN, nonce_hex, 2*NONCE_LEN, NULL, NULL, NULL);
  sodium_hex2bin(ciphertext, cipher_len, ciphertext_hex, 2*cipher_len, NULL, NULL, NULL);
  if (crypto_secretbox_open_easy(decrypted, ciphertext, cipher_len, nonce, key) != 0) {
    memset(key, 0, KEY_LEN);
    free(key);
    syslog(LOG_AUTHPRIV|LOG_DEBUG,"Decryption failed.");
    free(decrypted);
    /* If we get here, the Message was a forgery. This means someone (or the network) somehow tried to tamper with the message*/
    return NULL;
  }
  memset(key, 0, KEY_LEN);
  free(key);
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "Decrypted config is: %s\n",decrypted);
  return decrypted;
}

unsigned char* keyDerivation(const char* password, char salt_hex[2*SALT_LEN+1], int generateNewSalt) {
  unsigned char* key = calloc(sizeof(unsigned char), KEY_LEN+1);
    unsigned char salt[SALT_LEN];
  if(generateNewSalt) {
    /* Choose a random salt */
    randombytes_buf(salt, SALT_LEN);
  sodium_bin2hex(salt_hex, 2*SALT_LEN+1, salt, SALT_LEN);
  } else {
  sodium_hex2bin(salt, SALT_LEN, salt_hex, 2*SALT_LEN, NULL, NULL, NULL);
  }
  if (crypto_pwhash(key, KEY_LEN, password, strlen(password), salt,
        crypto_pwhash_OPSLIMIT_INTERACTIVE, crypto_pwhash_MEMLIMIT_INTERACTIVE,
        crypto_pwhash_ALG_DEFAULT) != 0) {
    syslog(LOG_AUTHPRIV|LOG_ALERT,"Could not derivate key. Probably because system out of memory.\n");
    return NULL;
  }
  return key;
}

// void writeNonceSalt(const char* filename) {
//   unsigned long cryptLen = conf_getCryptLen();
//   char* toWrite = malloc(SALT_LEN+NONCE_LEN+snprintf(NULL,0,"%lu",cryptLen)+1);
//   sprintf(toWrite, "%.*s%.*s%lu", NONCE_LEN, crypt.nonce, SALT_LEN, crypt.salt, cryptLen);
//   writeBufferToFile(filename, toWrite, SALT_LEN+NONCE_LEN+snprintf(NULL,0,"%lu",cryptLen));
//   free(toWrite);
// }
//
// void readNonceSalt(const char* filename) {
//   char* text = readFile(filename);
//   if(NULL==text) {
//     syslog(LOG_AUTHPRIV|LOG_NOTICE, "NonceFile '%s' could not be read\n",filename);
//     return;
//   }
//   unsigned char* nonce = calloc(sizeof(unsigned char), NONCE_LEN+1);
//   strncpy((char*)nonce, text, NONCE_LEN);
//   crypt.nonce = nonce;
//   unsigned char* salt = calloc(sizeof(unsigned char), SALT_LEN+1);
//   strncpy((char*)salt, text+NONCE_LEN, SALT_LEN);
//   crypt.salt = salt;
//   conf_setCryptLen(atoi(text+NONCE_LEN+SALT_LEN));
//   free(text);
// }
//

