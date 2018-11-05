#ifndef OIDC_REVOKE_H
#define OIDC_REVOKE_H

#include "account/account.h"
#include "utils/oidc_error.h"

oidc_error_t revokeToken(struct oidc_account* account);

#endif  // OIDC_REVOKE_H
