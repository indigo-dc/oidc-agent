#ifndef OIDC_REGISTRATION_H
#define OIDC_REGISTRATION_H

#include "../account.h"

char* dynamicRegistration(struct oidc_account* account,
                          int usePasswordGrantType, const char* access_token);

#endif
