#ifndef OIDC_JWK_H
#define OIDC_JWK_H

#include "utils/keySet.h"

#include <cjose/cjose.h>

void               secFreeJWK(cjose_jwk_t* jwk);
struct keySetPPstr createSigningKey();
cjose_jwk_t*       createRSAKey();
cjose_jwk_t*       import_jwk(const char* key);
cjose_jwk_t* import_jwk_fromURI(const char* jwk_uri, const char* cert_path,
                                const char* use);
cjose_jwk_t* import_jwk_enc_fromURI(const char* jwk_uri, const char* cert_path);
cjose_jwk_t* import_jwk_sign_fromURI(const char* jwk_uri,
                                     const char* cert_path);
char* export_jwk(const cjose_jwk_t* jwk, int withPrivate, const char* use);
char* export_jwk_sig(const cjose_jwk_t* jwk, int withPrivate);
char* export_jwk_enc(const cjose_jwk_t* jwk, int withPrivate);

#endif  // OIDC_JWK_H
