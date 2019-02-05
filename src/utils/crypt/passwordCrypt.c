#include "passwordCrypt.h"
#include "utils/crypt/cryptUtils.h"
#include "utils/numberString.h"

#include <sodium.h>

static const uint64_t maxSupportedPass = 692533995824480255;
static uint64_t       passwordPass;

void initPasswordCrypt() {
  uint32_t limit = maxSupportedPass - 0xffffffff;
  uint64_t a     = randombytes_uniform(limit);
  uint32_t b     = randombytes_random();
  uint64_t pass  = (a << 32) | b;
  passwordPass   = pass;
}

char* encryptPassword(const char* password) {
  if (password == NULL) {
    oidc_setArgNullFuncError(__func__);
    return NULL;
  }
  char* pass = numberToString(passwordPass);
  if (pass == NULL) {
    oidc_setInternalError("Password encryption password not retrievable");
    return NULL;
  }
  char* ret = crypt_encrypt(password, pass);
  secFree(pass);
  return ret;
}

char* decryptPassword(const char* cypher) {
  if (cypher == NULL) {
    // Don't set errno
    return NULL;
  }
  char* pass = numberToString(passwordPass);
  if (pass == NULL) {
    oidc_setInternalError("Password encryption password not retrievable");
    return NULL;
  }
  char* ret = crypt_decrypt(cypher, pass);
  secFree(pass);
  return ret;
}
