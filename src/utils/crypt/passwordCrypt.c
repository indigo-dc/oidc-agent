#include "passwordCrypt.h"

#include <sodium.h>

#include "utils/crypt/crypt.h"
#include "utils/memory.h"
#include "utils/numberString.h"
#include "utils/oidc_error.h"
#include "utils/stringUtils.h"

static const uint64_t maxSupportedPass = 692533995824480255;
static uint64_t       passwordPass;

void initPasswordCrypt() {
  uint64_t limit = maxSupportedPass - 0xffffffff;
  uint64_t a     = randombytes_uniform(limit);
  uint32_t b     = randombytes_random();
  uint64_t pass  = (a << 32) | b;
  passwordPass   = pass;
}

char* encryptPassword(const char* password, const char* salt) {
  if (password == NULL) {
    oidc_setArgNullFuncError(__func__);
    return NULL;
  }
  char* pass = numberToString(passwordPass);
  if (pass == NULL) {
    oidc_setInternalError("Password encryption password not retrievable");
    return NULL;
  }
  char* salted_pass = oidc_sprintf("%s%s", salt, pass);
  secFree(pass);
  char* ret = crypt_encrypt(password, salted_pass);
  secFree(salted_pass);
  return ret;
}

char* decryptPassword(const char* cypher, const char* salt) {
  if (cypher == NULL) {
    // Don't set errno
    return NULL;
  }
  char* pass = numberToString(passwordPass);
  if (pass == NULL) {
    oidc_setInternalError("Password encryption password not retrievable");
    return NULL;
  }
  char* salted_pass = oidc_sprintf("%s%s", salt, pass);
  secFree(pass);
  char* ret = crypt_decrypt(cypher, salted_pass);
  secFree(salted_pass);
  return ret;
}
