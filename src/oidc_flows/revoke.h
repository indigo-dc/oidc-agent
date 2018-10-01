#ifndef OIDC_REVOKE_H
#define OIDC_REVOKE_H

#include "../account.h"
#include "../oidc_error.h"

oidc_error_t revokeToken(struct oidc_account* account) ;

#endif // OIDC_REVOKE_H
