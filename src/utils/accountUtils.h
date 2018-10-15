#ifndef ACCOUNT_UTILS_H
#define ACCOUNT_UTILS_H

#include "../account.h"

#include "../../lib/list/src/list.h"

#include <time.h>

time_t               getMinDeath(list_t* accounts);
struct oidc_account* getDeathAccount(list_t* accounts);

#endif  // ACCOUNT_UTILS_H
