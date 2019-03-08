#ifndef ACCOUNT_UTILS_H
#define ACCOUNT_UTILS_H

#include "account/account.h"

#include <time.h>

time_t               getMinAccountDeath();
struct oidc_account* getDeathAccount();

#endif  // ACCOUNT_UTILS_H
