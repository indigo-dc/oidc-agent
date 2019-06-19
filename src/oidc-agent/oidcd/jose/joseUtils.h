#ifndef OIDC_JOSE_UTILS_H
#define OIDC_JOSE_UTILS_H

#include "account/account.h"

#include "../../../cjose/include/cjose/cjose.h"

void               initCJOSE();
cjose_header_t*    cjose_getSignHeader(const char* sign_alg);
void               secFreeJoseHeader(cjose_header_t* hdr);
struct keySetSEstr createRSAKeys(struct oidc_account* a);
char* jose_sign(const char* plain, const struct oidc_account* account);

#endif  // OIDC_JOSE_UTILS_H
