#ifndef OIDC_JWK_H
#define OIDC_JWK_H

#include <cjose/cjose.h>

struct strKeySet {
  char* priv;
  char* pub;
};

void             secFreeJWK(cjose_jwk_t* jwk);
struct strKeySet createSigningKey();
cjose_jwk_t*     createRSAKey();
cjose_jwk_t*     import_jwk(const char* key);
char* export_jwk(const cjose_jwk_t* jwk, int withPrivate, const char* use);
char* export_jwk_sig(const cjose_jwk_t* jwk, int withPrivate);
char* export_jwk_enc(const cjose_jwk_t* jwk, int withPrivate);

#endif  // OIDC_JWK_H
