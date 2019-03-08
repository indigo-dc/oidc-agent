#include "accountUtils.h"
#include "account/account.h"
#include "deathUtils.h"
#include "utils/db/account_db.h"

#include <syslog.h>
#include <time.h>

/**
 * @brief returns the minimum death time in an account list
 * @param accounts a list of (loaded) accounts
 * @return the minimum time of death; might be @c 0
 */
time_t getMinAccountDeath() {
  syslog(LOG_AUTHPRIV | LOG_DEBUG, "Getting min death time for accounts");
  return accountDB_getMinDeath((time_t(*)(void*))account_getDeath);
}

/**
 * @brief returns an account that death was prior to the current time
 * @param accounts a list of (loaded) accounts - searchspace
 * only one death account is returned per call; to find all death accounts in @p
 * accounts @c getDeathAccount should be called until it returns @c NULL
 * @return a pointer to a dead account or @c NULL
 */
struct oidc_account* getDeathAccount() {
  syslog(LOG_AUTHPRIV | LOG_DEBUG, "Searching for death accounts");
  return accountDB_getDeathEntry((time_t(*)(void*))account_getDeath);
}
