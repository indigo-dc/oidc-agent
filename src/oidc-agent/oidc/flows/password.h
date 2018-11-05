#ifndef OIDC_PASSWORD_H
#define OIDC_PASSWORD_H

#include "account/account.h"
#include "utils/oidc_error.h"

oidc_error_t passwordFlow(struct oidc_account* p);

#endif  // OIDC_PASSWORD_H
