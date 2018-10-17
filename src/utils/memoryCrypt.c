#include "memoryCrypt.h"
#include "cryptUtils.h"
#include "memory.h"
#include "numberString.h"

#include <sodium.h>

uint64_t memoryPass;

uint64_t maxSupportedPass = 692533995824480255;

void initMemoryCrypt() {
  uint32_t limit = maxSupportedPass - 0xffffffff;
  uint64_t a     = randombytes_uniform(limit);
  uint32_t b     = randombytes_random();
  uint64_t pass  = (a << 32) | b;
  memoryPass     = pass;
}

char* memoryEncrypt(char* str) {
  if (str == NULL) {
    return NULL;
  }
  char* pass      = numberToString(memoryPass);
  char* encrypted = encryptText(str, pass);
  secFree(pass);
  secFree(str);
  return encrypted;
}

char* memoryDecrypt(char* str) {
  if (str == NULL) {
    return NULL;
  }
  char* pass      = numberToString(memoryPass);
  char* decrypted = (char*)decryptText(str, pass);
  secFree(pass);
  return decrypted;
}
