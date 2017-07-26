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

struct {
  unsigned char* nonce;
  unsigned char* salt;
} crypt;

// void printHex(unsigned char* text, int len) {
//   int i=0;
//   char* toPrint = calloc(1,len*2+1);
//   char* printIt = toPrint;
//   for(i=0;i<len;i++) {
//     toPrint += sprintf(toPrint, "%02X", text[i]);
//   }
//   syslog(LOG_DEBUG|LOG_AUTHPRIV, "%s\n",printIt);
//   free(printIt);
// }

unsigned char* encrypt(const unsigned char* text, const char* password) {
  unsigned char* nonce = calloc(sizeof(unsigned char), NONCE_LEN+1);
  randombytes_buf(nonce, NONCE_LEN);
  conf_setCryptLen(strlen((char*)text));
  unsigned char* ciphertext = calloc(sizeof(unsigned char), MAC_LEN +conf_getCryptLen() +1);
  unsigned char* key = keyDerivation(password);
  // syslog(LOG_AUTHPRIV|LOG_DEBUG,"encrypt salt: ");
  // printHex(conf_getSalt(), SALT_LEN);
  // syslog(LOG_AUTHPRIV|LOG_DEBUG,"encrypt key: ");
  // printHex(key, KEY_LEN);
  crypto_secretbox_easy(ciphertext, text, strlen((char*)text), nonce, key);
  memset(key, 0, KEY_LEN);
  free(key);
  crypt.nonce = nonce;
  // syslog(LOG_AUTHPRIV|LOG_DEBUG,"encrpt nonce: ");
  // printHex(conf_getNonce(), NONCE_LEN);
  // syslog(LOG_AUTHPRIV|LOG_DEBUG,"encrpt cipher: ");
  // printHex(ciphertext, MAC_LEN +conf_getCryptLen());
  return ciphertext;
}

unsigned char* decrypt(const unsigned char* ciphertext, const char* password) {
  syslog(LOG_AUTHPRIV|LOG_DEBUG,"decrypt size is: %lu\n", conf_getCryptLen());
  unsigned char* decrypted = calloc(sizeof(unsigned char), conf_getCryptLen()+1);
  unsigned char* key = keyDerivation(password);
  // syslog(LOG_AUTHPRIV|LOG_DEBUG,"decrypt salt: ");
  // printHex(conf_getSalt(), SALT_LEN);
  // syslog(LOG_AUTHPRIV|LOG_DEBUG,"decrypt key: ");
  // printHex(key, KEY_LEN);
  // syslog(LOG_AUTHPRIV|LOG_DEBUG,"decrpt nonce: ");
  // printHex(conf_getNonce(), NONCE_LEN);
  // syslog(LOG_AUTHPRIV|LOG_DEBUG,"decrpt cipher: ");
  // printHex((unsigned char*)ciphertext, MAC_LEN +conf_getCryptLen());
  if (crypto_secretbox_open_easy(decrypted, ciphertext, MAC_LEN +conf_getCryptLen(), crypt.nonce, key) != 0) {
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

unsigned char* keyDerivation(const char* password) {
  unsigned char* key = calloc(sizeof(unsigned char), KEY_LEN+1);
  if(crypt.salt==NULL) {
    crypt.salt = calloc(sizeof(unsigned char), SALT_LEN+1);
    /* Choose a random salt */
    randombytes_buf(crypt.salt, SALT_LEN);
  }
  if (crypto_pwhash(key, KEY_LEN, password, strlen(password), crypt.salt,
        crypto_pwhash_OPSLIMIT_INTERACTIVE, crypto_pwhash_MEMLIMIT_INTERACTIVE,
        crypto_pwhash_ALG_DEFAULT) != 0) {
    syslog(LOG_AUTHPRIV|LOG_ALERT,"Could not derivate key. Probably because system out of memory.\n");
    return NULL;
  }
  return key;
}

void writeNonceSalt(const char* filename) {
  unsigned long cryptLen = conf_getCryptLen();
  char* toWrite = malloc(SALT_LEN+NONCE_LEN+snprintf(NULL,0,"%lu",cryptLen)+1);
  sprintf(toWrite, "%.*s%.*s%lu", NONCE_LEN, crypt.nonce, SALT_LEN, crypt.salt, cryptLen);
  writeBufferToFile(filename, toWrite, SALT_LEN+NONCE_LEN+snprintf(NULL,0,"%lu",cryptLen));
  free(toWrite);
}

void readNonceSalt(const char* filename) {
  char* text = readFile(filename);
  if(NULL==text) {
    syslog(LOG_AUTHPRIV|LOG_NOTICE, "NonceFile '%s' could not be read\n",filename);
    return;
  }
  unsigned char* nonce = calloc(sizeof(unsigned char), NONCE_LEN+1);
  strncpy((char*)nonce, text, NONCE_LEN);
  crypt.nonce = nonce;
  unsigned char* salt = calloc(sizeof(unsigned char), SALT_LEN+1);
  strncpy((char*)salt, text+NONCE_LEN, SALT_LEN);
  crypt.salt = salt;
  conf_setCryptLen(atoi(text+NONCE_LEN+SALT_LEN));
  free(text);
}


