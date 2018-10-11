#ifndef OIDC_REFRESH_H
#define OIDC_REFRESH_H

#include "../account.h"

char* refreshFlow(struct oidc_account* p, const char* scope);

#endif  // OIDC_REFRESH_H
