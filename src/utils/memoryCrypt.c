#include "memoryCrypt.h"
#include "../oidc_error.h"
#include "memory.h"
#include "stringUtils.h"

#include <string.h>

#include <sodium.h>

uint64_t memoryPass;

// uint64_t maxSupportedPass = 692533995824480255;

char* xorCrypt(const char* string, uint64_t key, size_t len) {
  size_t length = sizeof(key);
  char*  str    = secAlloc(len + 1);
  char*  s      = str;
  for (size_t i = 0; i < len; i++) {
    *str = *string ^ *((unsigned char*)&key + (i % length));
    str++;
    string++;
  }
  return s;
}

char* memoryDecrypt(const char* cipher) {
  if (cipher == NULL) {
    oidc_setArgNullFuncError(__func__);
    return NULL;
  }
  char*          fileText   = oidc_strcopy(cipher);
  size_t         len        = atoi(strtok(fileText, ":"));
  char*          cipher_hex = strtok(NULL, ":");
  unsigned char* cipher_bin = secAlloc(sizeof(char) * (len + 1));
  sodium_hex2bin(cipher_bin, len + 1, cipher_hex, 2 * len + 1, NULL, NULL,
                 NULL);
  char* decrypted = xorCrypt((char*)cipher_bin, memoryPass, len);
  secFree(cipher_bin);
  secFree(fileText);
  return decrypted;
}

char* memoryEncrypt(const char* text) {
  size_t len        = strlen(text);
  char*  cipher     = xorCrypt(text, memoryPass, len);
  char*  cipher_hex = secAlloc(sizeof(char) * (2 * len + 1));
  sodium_bin2hex(cipher_hex, 2 * len + 1, (unsigned char*)cipher, len);
  secFree(cipher);
  char* fmt      = "%lu:%s";
  char* ciphered = oidc_sprintf(fmt, len, cipher_hex);
  secFree(cipher_hex);
  return ciphered;
}

void initMemoryCrypt() {
  // uint32_t limit = maxSupportedPass - 0xffffffff;
  // uint64_t a     = randombytes_uniform(limit);
  uint64_t a    = randombytes_random();
  uint32_t b    = randombytes_random();
  uint64_t pass = (a << 32) | b;
  memoryPass    = pass;
}
