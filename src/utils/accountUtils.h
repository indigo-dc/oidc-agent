#ifndef ACCOUNT_UTILS_H
#define ACCOUNT_UTILS_H

#include "account/account.h"

#include "list/list.h"

#include <time.h>

time_t               getMinDeath(list_t* accounts);
struct oidc_account* getDeathAccount(list_t* accounts);

#endif  // ACCOUNT_UTILS_H
