#define _XOPEN_SOURCE 500
#include "lock_state.h"

#include "agent_state.h"
#include "utils/crypt/cryptUtils.h"
#include "utils/memory.h"
#include "utils/oidc_error.h"
#include "utils/sleeper.h"

#include <string.h>
#include <syslog.h>
#include <unistd.h>

oidc_error_t unlock(const char* password) {
  static unsigned char fail_count = 0;
  syslog(LOG_AUTHPRIV | LOG_DEBUG, "Unlocking agent");
  if (agent_state.lock_state.locked == 0) {
    syslog(LOG_AUTHPRIV | LOG_DEBUG, "Agent not locked");
    oidc_errno = OIDC_ENOTLOCKED;
    return oidc_errno;
  }

  if (lockDecrypt(password) == OIDC_SUCCESS) {
    agent_state.lock_state.locked = 0;
    fail_count                    = 0;
    syslog(LOG_AUTHPRIV | LOG_DEBUG, "Agent unlocked");
    return OIDC_SUCCESS;
  }
  /* delay in 0.1s increments up to 10s */
  if (fail_count < 100) {
    fail_count++;
  }
  unsigned int delay = 100 * fail_count;
  syslog(LOG_AUTHPRIV | LOG_DEBUG, "unlock failed, delaying %0.1lf seconds",
         (double)delay / 1000);
  msleep(delay);
  return oidc_errno;
}

oidc_error_t lock(const char* password) {
  syslog(LOG_AUTHPRIV | LOG_DEBUG, "Locking agent");
  if (agent_state.lock_state.locked) {
    syslog(LOG_AUTHPRIV | LOG_DEBUG, "Agent already locked");
    oidc_errno = OIDC_ELOCKED;
    return oidc_errno;
  }
  if (lockEncrypt(password) != OIDC_SUCCESS) {
    return oidc_errno;
  }
  agent_state.lock_state.locked = 1;
  syslog(LOG_AUTHPRIV | LOG_DEBUG, "Agent locked");
  return OIDC_SUCCESS;
}
