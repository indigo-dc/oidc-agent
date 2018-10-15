#include "lock_state.h"

#include "agent_state.h"
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
  unsigned char* hashedPw =
      crypt_keyDerivation(password, agent_state.lock_state.salt_hex, 0);
  if (crypt_compare(hashedPw, agent_state.lock_state.hashedPw)) {
    agent_state.lock_state.locked = 0;
    memset(agent_state.lock_state.salt_hex, 0, 2 * SALT_LEN + 1);  // memset
    secFree(agent_state.lock_state.hashedPw);
    syslog(LOG_AUTHPRIV | LOG_DEBUG, "Agent unlocked");
    secFree(hashedPw);
    return OIDC_SUCCESS;
  }
  secFree(hashedPw);
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
  unsigned char* hashedPw =
      crypt_keyDerivation(password, agent_state.lock_state.salt_hex, 1);
  if (hashedPw != NULL) {
    agent_state.lock_state.locked   = 1;
    agent_state.lock_state.hashedPw = hashedPw;
    syslog(LOG_AUTHPRIV | LOG_DEBUG, "Agent locked");
  }
  return OIDC_SUCCESS;
}
