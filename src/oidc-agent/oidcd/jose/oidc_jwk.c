#include "oidc_jwk.h"
#include "defines/settings.h"
#include "utils/logger.h"
#include "utils/memory.h"
#include "utils/oidc_error.h"
#include "utils/stringUtils.h"

#include <cjose/cjose.h>

const uint8_t* e    = (uint8_t*)"\x01\x00\x01";
const size_t   elen = 3;

char* export_jwk(const cjose_jwk_t* jwk, int withPrivate) {
  if (jwk == NULL) {
    oidc_setArgNullFuncError(__func__);
    return NULL;
  }
  cjose_err err;
  char*     tmp = cjose_jwk_to_json(jwk, withPrivate, &err);
  if (tmp == NULL) {
    char* err_msg = oidc_sprintf("JWK export error: %s", err.message);
    oidc_setInternalError(err_msg);
    logger(ERROR, err_msg);
    secFree(err_msg);
    return NULL;
  }
  // TODO remove and add stuff
  // this might depend on the type
  return jwk_str;
}

cjose_jwk_t* import_jwk(const char* key) {
  if (key == NULL) {
    oidc_setArgNullFuncError(__func__);
    return NULL;
  }
  cjose_err    err;
  cjose_jwk_t* jwk = cjose_jwk_import(key, strlen(key), &err);
  if (jwk == NULL) {
    char* err_msg = oidc_sprintf("JWK import error: %s", err.message);
    oidc_setInternalError(err_msg);
    logger(ERROR, err_msg);
    secFree(err_msg);
    return NULL;
  }
  return jwk;
}

cjose_jwk_t* import_jwk_fromURI(const char* jwk_uri) {
  // TODO
  return NULL;
}

cjose_jwk_t* createRSAKey() {
  cjose_err    err;
  cjose_jwk_t* jwk = cjose_jwk_create_RSA_random(RSA_KEY_BITS, e, elen, &err);
  if (jwk == NULL) {
    char* err_msg =
        oidc_sprintf("Error while creationg signing key: %s", err.message);
    oidc_setInternalError(err_msg);
    logger(ERROR, err_msg);
    secFree(err_msg);
    return NULL;
  }
  return jwk;
}

struct strKeySet createSigningKey() {
  cjose_jwk_t* jwk      = createRSAKey();
  char*        key_priv = export_jwk(jwk, 1);
  char*        key_pub  = export_jwk(jwk, 0);
  secFreeJWK(jwk);
  return (struct strKeySet){.priv = key_priv, .pub = key_pub};
}

void secFreeJWK(cjose_jwk_t* jwk) { cjose_jwk_release(jwk); }
