#ifndef OIDC_TOKEN_HANDLER_H
#define OIDC_TOKEN_HANDLER_H

#include "oidc-token_options.h"

void token_handleIdToken(const unsigned char useIssuerInsteadOfShortname,
                         const char*         name);

#endif /* OIDC_TOKEN_HANDLER_H */
