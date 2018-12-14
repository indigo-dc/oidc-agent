#include "memoryCrypt.h"
#include "crypt.h"
#include "utils/memory.h"
#include "utils/oidc_error.h"
#include "utils/stringUtils.h"

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
  char*          tmp           = oidc_strcopy(cipher);
  size_t         len           = strToInt(strtok(tmp, ":"));
  char*          cipher_base64 = strtok(NULL, ":");
  unsigned char* cipher_bin    = secAlloc(sizeof(char) * (len + 1));
  fromBase64(cipher_base64, len, cipher_bin);
  char* decrypted = xorCrypt((char*)cipher_bin, memoryPass, len);
  secFree(cipher_bin);
  secFree(tmp);
  return decrypted;
}

char* memoryEncrypt(const char* text) {
  size_t len           = strlen(text);
  char*  cipher        = xorCrypt(text, memoryPass, len);
  char*  cipher_base64 = toBase64(cipher, len);
  secFree(cipher);
  char* fmt      = "%lu:%s";
  char* ciphered = oidc_sprintf(fmt, len, cipher_base64);
  secFree(cipher_base64);
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
