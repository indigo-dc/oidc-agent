#ifndef OIDC_CODE_H
#define OIDC_CODE_H

#include "account/account.h"
#include "utils/oidc_error.h"

char*        buildCodeFlowUri(const struct oidc_account* account, char* state);
oidc_error_t codeExchange(struct oidc_account* account, const char* code,
                          const char* used_redirect_uri);

#endif  // OIDC_CODE_H
