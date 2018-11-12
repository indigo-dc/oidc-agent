#ifndef OIDC_REGISTRATION_H
#define OIDC_REGISTRATION_H

#include "account/account.h"
#include "list/list.h"

char* dynamicRegistration(struct oidc_account* account, list_t* flows,
                          const char* access_token);

#endif
