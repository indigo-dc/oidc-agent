#include "oidc_jwk.h"
#include "defines/settings.h"
#include "oidc-agent/http/http_ipc.h"
#include "utils/json.h"
#include "utils/listUtils.h"
#include "utils/logger.h"
#include "utils/memory.h"
#include "utils/oidc_error.h"
#include "utils/stringUtils.h"

const uint8_t* e    = (uint8_t*)"\x01\x00\x01";
const size_t   elen = 3;

#define JWK_USE_SIG "sig"
#define JWK_USE_ENC "enc"

char* export_jwk_sig(const cjose_jwk_t* jwk, int withPrivate) {
  return export_jwk(jwk, withPrivate, JWK_USE_SIG);
}

char* export_jwk_enc(const cjose_jwk_t* jwk, int withPrivate) {
  return export_jwk(jwk, withPrivate, JWK_USE_ENC);
}

char* export_jwk(const cjose_jwk_t* jwk, int withPrivate, const char* use) {
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
  cJSON* json = stringToJson(tmp);
  secFree(tmp);
  jsonAddStringValue(json, "use", use);
  char* jwk_str = jsonToStringUnformatted(json);
  secFreeJson(json);
  logger(DEBUG, "Exported jwk is '%s'", jwk_str);
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

cjose_jwk_t* import_jwk_sign_fromURI(const char* jwk_uri,
                                     const char* cert_path) {
  return import_jwk_fromURI(jwk_uri, cert_path, JWK_USE_SIG);
}

cjose_jwk_t* import_jwk_enc_fromURI(const char* jwk_uri,
                                    const char* cert_path) {
  return import_jwk_fromURI(jwk_uri, cert_path, JWK_USE_ENC);
}

cjose_jwk_t* import_jwk_fromURI(const char* jwk_uri, const char* cert_path,
                                const char* use) {
  if (jwk_uri == NULL || cert_path == NULL) {
    oidc_setArgNullFuncError(__func__);
    return NULL;
  }
  char* res = httpsGET(jwk_uri, NULL, cert_path);
  if (res == NULL) {
    return NULL;
  }
  char* keys_arr = getJSONValueFromString(res, "keys");
  secFree(res);
  if (keys_arr == NULL) {
    oidc_errno = OIDC_EJWKURINO;
    return NULL;
  }
  list_t* keys = JSONArrayStringToList(keys_arr);
  secFree(keys_arr);
  if (!listValid(keys)) {
    oidc_errno = OIDC_EJWKURINO;
    secFreeList(keys);
    return NULL;
  }
  char* jwk = NULL;
  if (keys->len == 1) {
    jwk = list_at(keys, 0)->val;
  } else {
    // TODO determine correct key -> depends on the purpose
    oidc_errno = OIDC_NOTIMPL;
    secFreeList(keys);
    return NULL;
  }
  cjose_jwk_t* imported = import_jwk(jwk);
  secFreeList(keys);
  return imported;
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

struct keySetPPstr createSigningKey() {
  cjose_jwk_t* jwk      = createRSAKey();
  char*        key_priv = export_jwk_sig(jwk, 1);
  char*        key_pub  = export_jwk_sig(jwk, 0);
  secFreeJWK(jwk);
  return (struct keySetPPstr){.priv = key_priv, .pub = key_pub};
}

struct keySetPPstr createEncryptionKey() {
  cjose_jwk_t* jwk      = createRSAKey();
  char*        key_priv = export_jwk_enc(jwk, 1);
  char*        key_pub  = export_jwk_enc(jwk, 0);
  secFreeJWK(jwk);
  return (struct keySetPPstr){.priv = key_priv, .pub = key_pub};
}

void secFreeJWK(cjose_jwk_t* jwk) { cjose_jwk_release(jwk); }
