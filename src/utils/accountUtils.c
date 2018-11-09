#include "accountUtils.h"

#include <syslog.h>
#include <time.h>

time_t getMinDeath(list_t* accounts) {
  syslog(LOG_AUTHPRIV | LOG_DEBUG, "Getting min death time for accounts");
  time_t           min = 0;
  list_node_t*     node;
  list_iterator_t* it = list_iterator_new(accounts, LIST_HEAD);
  while ((node = list_iterator_next(it))) {
    struct oidc_account* acc   = node->val;
    time_t               death = account_getDeath(*acc);
    syslog(LOG_AUTHPRIV | LOG_DEBUG, "this death is %lu", death);
    if (death > 0 && (death < min || min == 0) && death > time(NULL)) {
      syslog(LOG_AUTHPRIV | LOG_DEBUG, "updating min to %lu", death);
      min = death;
    }
  }
  list_iterator_destroy(it);
  syslog(LOG_AUTHPRIV | LOG_DEBUG, "Minimum death in account list is %lu", min);
  return min;
}

struct oidc_account* getDeathAccount(list_t* accounts) {
  syslog(LOG_AUTHPRIV | LOG_DEBUG, "Searching for death accounts");
  list_node_t*     node;
  list_iterator_t* it = list_iterator_new(accounts, LIST_HEAD);
  while ((node = list_iterator_next(it))) {
    struct oidc_account* acc   = node->val;
    time_t               death = account_getDeath(*acc);
    time_t               now   = time(NULL);
    if (death > 0 && death <= now) {
      list_iterator_destroy(it);
      syslog(LOG_AUTHPRIV | LOG_DEBUG,
             "Found account died at %lu (current time %lu)", death, now);
      return acc;
    }
  }
  list_iterator_destroy(it);
  syslog(LOG_AUTHPRIV | LOG_DEBUG, "Found no death account");
  return NULL;
}
