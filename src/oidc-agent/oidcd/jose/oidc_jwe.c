#include "oidc_jwe.h"
#include "oidc-agent/oidcd/jose/joseUtils.h"
#include "utils/logger.h"
#include "utils/oidc_error.h"

#include <string.h>

char* jwe_encrypt(const char* plain, cjose_jwk_t* jwk, const char* alg,
                  const char* enc) {
  if (plain == NULL || jwk == NULL || alg == NULL || enc == NULL) {
    logger(DEBUG, "plain %s", plain);
    logger(DEBUG, "jwk %p", jwk);
    logger(DEBUG, "alg %s", alg);
    oidc_setArgNullFuncError(__func__);
    return NULL;
  }
  cjose_err       err;
  cjose_header_t* hdr = cjose_getEncryptHeader(alg, enc);
  cjose_jwe_t*    jwe =
      cjose_jwe_encrypt(jwk, hdr, (uint8_t*)plain, strlen(plain), &err);
  secFreeJoseHeader(hdr);
  if (jwe == NULL) {
    char* err_msg = oidc_sprintf("jwe error: %s", err.message);
    oidc_setInternalError(err_msg);
    logger(ERROR, err_msg);
    secFree(err_msg);
    return NULL;
  }

  char* ret = export_jwe(jwe);
  secFreeJWE(jwe);
  return ret;
}

char* jwe_decrypt(const char* crypt_msg, cjose_jwk_t* jwk) {
  if (crypt_msg == NULL || jwk == NULL) {
    oidc_setArgNullFuncError(__func__);
    return NULL;
  }
  cjose_err    err;
  cjose_jwe_t* jwe = import_jwe(crypt_msg);
  if (jwe == NULL) {
    return NULL;
  }
  size_t         size  = 0;
  unsigned char* crypt = cjose_jwe_decrypt(jwe, jwk, &size, &err);
  secFreeJWE(jwe);
  if (crypt == NULL) {
    oidc_errno = OIDC_EJWEDECRY;
    return NULL;
  }
  return (char*)crypt;
}

cjose_jwe_t* import_jwe(const char* crypt) {
  if (crypt == NULL) {
    oidc_setArgNullFuncError(__func__);
    return NULL;
  }
  cjose_err    err;
  cjose_jwe_t* jwe = cjose_jwe_import(crypt, strlen(crypt), &err);
  if (jwe == NULL) {
    char* err_msg = oidc_sprintf("jws import error: %s", err.message);
    oidc_setInternalError(err_msg);
    logger(ERROR, err_msg);
    secFree(err_msg);
    return NULL;
  }
  return jwe;
}

char* export_jwe(cjose_jwe_t* jwe) {
  if (jwe == NULL) {
    oidc_setArgNullFuncError(__func__);
    return NULL;
  }
  cjose_err err;
  char*     crypt = cjose_jwe_export(jwe, &err);
  if (crypt == NULL) {
    char* err_msg = oidc_sprintf("jws export error: %s", err.message);
    oidc_setInternalError(err_msg);
    logger(ERROR, err_msg);
    secFree(err_msg);
    return NULL;
  }
  return crypt;
}

void secFreeJWE(cjose_jwe_t* jwe) { cjose_jwe_release(jwe); }
