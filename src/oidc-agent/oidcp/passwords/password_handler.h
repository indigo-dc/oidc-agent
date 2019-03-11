#ifndef OIDC_PASSWORD_HANDLER_H
#define OIDC_PASSWORD_HANDLER_H

#include "utils/lifetimeArg.h"
#include "utils/oidc_error.h"

oidc_error_t pw_handleSave(const char*              pw_entry_str,
                           const struct lifetimeArg pw_lifetime);
#endif  // OIDC_PASSWORD_HANDLER_H
