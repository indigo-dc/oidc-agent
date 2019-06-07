#ifndef OIDC_JOSE_UTILS_H
#define OIDC_JOSE_UTILS_H

#include <cjose/cjose.h>

void            initCJOSE();
cjose_header_t* cjose_getSignHeader(const char* sign_alg);
void            secFreeJoseHeader(cjose_header_t* hdr);

#endif  // OIDC_JOSE_UTILS_H
