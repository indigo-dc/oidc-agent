#ifndef ACCOUNT_UTILS_H
#define ACCOUNT_UTILS_H

#include "account/account.h"
#include "list/list.h"

#include <time.h>

time_t               getMinAccountDeath(list_t* accounts);
struct oidc_account* getDeathAccount(list_t* accounts);
struct oidc_account* accountFromFile(const char* filename);

#endif  // ACCOUNT_UTILS_H
