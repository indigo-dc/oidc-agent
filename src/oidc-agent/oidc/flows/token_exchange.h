#ifndef OIDC_TOKEN_EXCHANGE
#define OIDC_TOKEN_EXCHANGE

#include "account/account.h"
#include "utils/oidc_error.h"

oidc_error_t tokenExchange(struct oidc_account* account);

#endif  // OIDC_TOKEN_EXCHANGE
