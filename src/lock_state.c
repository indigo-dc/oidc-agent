#include "lock_state.h"

#include "agent_state.h"
#include "utils/cryptUtils.h"
#include "utils/memory.h"

#include <string.h>
#include <syslog.h>

oidc_error_t unlock(const char* password) {
  syslog(LOG_AUTHPRIV | LOG_DEBUG, "Unlocking agent");
  if (agent_state.lock_state.locked == 0) {
    syslog(LOG_AUTHPRIV | LOG_DEBUG, "Agent not locked");
    oidc_errno = OIDC_ENOTLOCKED;
    return oidc_errno;
  }

  if (compareToHash(password, agent_state.lock_state.hash)) {
    agent_state.lock_state.locked = 0;
    secFreeHashed(agent_state.lock_state.hash);
    syslog(LOG_AUTHPRIV | LOG_DEBUG, "Agent unlocked");
    return OIDC_SUCCESS;
  }
  oidc_errno = OIDC_EPASS;
  return oidc_errno;
}

oidc_error_t lock(const char* password) {
  syslog(LOG_AUTHPRIV | LOG_DEBUG, "Locking agent");
  if (agent_state.lock_state.locked) {
    syslog(LOG_AUTHPRIV | LOG_DEBUG, "Agent already locked");
    oidc_errno = OIDC_ELOCKED;
    return oidc_errno;
  }
  lock_state_setHash(&(agent_state.lock_state), hash(password));
  if (agent_state.lock_state.hash->hash != NULL) {
    agent_state.lock_state.locked = 1;
    syslog(LOG_AUTHPRIV | LOG_DEBUG, "Agent locked");
  }
  return OIDC_SUCCESS;
}

void lock_state_setHash(struct lock_state* l, struct hashed* h) {
  secFree(l->hash);
  l->hash = h;
}
