#include "password_handler.h"

#include "oidc-agent/oidcp/passwords/password_store.h"
#include "utils/agentLogger.h"
#include "utils/oidc_error.h"
#include "utils/password_entry.h"

oidc_error_t pw_handleSave(const char* pw_entry_str) {
  if (pw_entry_str == NULL) {
    oidc_setArgNullFuncError(__func__);
    return oidc_errno;
  }
  struct password_entry* pw = JSONStringToPasswordEntry(pw_entry_str);
  if (!pw->shortname) {
    oidc_setInternalError("shortname not set in pw_entry");
    agent_log(ERROR, "%s", oidc_serror());
    return oidc_errno;
  }
  if (!pw->type) {
    oidc_setInternalError("type not set in pw_entry");
    agent_log(ERROR, "%s", oidc_serror());
    return oidc_errno;
  }
  if (!pw->password && (pw->type & PW_TYPE_MEM)) {
    oidc_setInternalError("password not set in pw_entry");
    agent_log(ERROR, "%s", oidc_serror());
    return oidc_errno;
  }
  return savePassword(pw);
}
