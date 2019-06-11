#ifndef OIDC_JWS_H
#define OIDC_JWS_H

#include "utils/oidc_error.h"

#include <cjose/cjose.h>

char*        export_jws(cjose_jws_t* jws);
cjose_jws_t* import_jws(const char* sign);
void         secFreeJWS(cjose_jws_t* jws);
oidc_error_t jws_verify(const char* signed_msg, cjose_jwk_t* jwk);
char*        jws_sign(const char* plain, cjose_jwk_t* jwk, const char* alg);

#endif  // OIDC_JWS_H
