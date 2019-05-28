#include "password_handler.h"
#include "oidc-agent/oidcp/passwords/password_store.h"
#include "utils/logger.h"
#include "utils/oidc_error.h"
#include "utils/password_entry.h"

oidc_error_t pw_handleSave(const char*              pw_entry_str,
                           const struct lifetimeArg pw_lifetime) {
  if (pw_entry_str == NULL) {
    oidc_setArgNullFuncError(__func__);
    return oidc_errno;
  }
  struct password_entry* pw = JSONStringToPasswordEntry(pw_entry_str);
  if (pwe_getExpiresAt(pw) == 0 && pw_lifetime.argProvided) {
    pwe_setExpiresIn(pw, pw_lifetime.lifetime);
  }
  if (!pw->shortname) {
    oidc_setInternalError("shortname not set in pw_entry");
    logger(ERROR, "%s", oidc_serror());
    return oidc_errno;
  }
  if (!pw->type) {
    oidc_setInternalError("type not set in pw_entry");
    logger(ERROR, "%s", oidc_serror());
    return oidc_errno;
  }
  if (!pw->password && (pw->type & (PW_TYPE_MNG | PW_TYPE_MEM))) {
    oidc_setInternalError("password not set in pw_entry");
    logger(ERROR, "%s", oidc_serror());
    return oidc_errno;
  }
  return savePassword(pw);
}
