#ifndef OIDC_JWE_H
#define OIDC_JWE_H

#include "../../../cjose/include/cjose/cjose.h"

void         secFreeJWE(cjose_jwe_t* jwe);
char*        export_jwe(cjose_jwe_t* jwe);
cjose_jwe_t* import_jwe(const char* crypt);
char*        jwe_decrypt(const char* crypt_msg, cjose_jwk_t* jwk);
char*        jwe_encrypt(const char* plain, cjose_jwk_t* jwk, const char* alg,
                         const char* enc);

#endif  // OIDC_JWE_H
