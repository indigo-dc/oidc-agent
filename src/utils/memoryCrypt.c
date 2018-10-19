#include "memoryCrypt.h"
#include "../oidc_error.h"
#include "memory.h"
#include "stringUtils.h"

#include <string.h>

#include <sodium.h>

uint64_t memoryPass;

uint64_t maxSupportedPass = 692533995824480255;

char* xorCrypt(const char* string, uint64_t key, size_t len) {
  size_t length = sizeof(key);
  char*  str    = secAlloc(len + 1);
  for (size_t i = 0; i < len; i++) {
    *str++ = *string++ ^ *((unsigned char*)&key + (i % length));
  }
  return str;
}

char* memoryDecrypt(const char* cipher) {
  if (cipher == NULL) {
    oidc_setArgNullFuncError(__func__);
    return NULL;
  }
  char*  fileText   = oidc_strcopy(cipher);
  size_t len        = atoi(strtok(fileText, ":"));
  char*  cipher_tex = strtok(NULL, ":");
  char*  decrypted  = xorCrypt(cipher_tex, memoryPass, len);
  secFree(fileText);
  return decrypted;
}

char* memoryEncrypt(const char* text) {
  size_t len        = strlen(text);
  char*  cipher_hex = xorCrypt(text, memoryPass, len);
  char*  fmt        = "%lu:%s";
  char*  cipher     = oidc_sprintf(fmt, len, cipher_hex);
  secFree(cipher_hex);
  return cipher;
}

void initMemoryCrypt() {
  uint32_t limit = maxSupportedPass - 0xffffffff;
  uint64_t a     = randombytes_uniform(limit);
  uint32_t b     = randombytes_random();
  uint64_t pass  = (a << 32) | b;
  memoryPass     = pass;
}
