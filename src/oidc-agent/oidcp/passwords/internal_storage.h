#ifndef OIDCAGENT_PASSWORD_STORAGE_H
#define OIDCAGENT_PASSWORD_STORAGE_H

#include "utils/oidc_error.h"

#include <time.h>

oidc_error_t internal_savePasswordFor(const char* shortname,
                                      const char* password, time_t expires_at);
char*        internal_getPasswordFor(const char* shortname);
oidc_error_t internal_removePasswordFor(const char* shortname);

#endif  // OIDCAGENT_PASSWORD_STORAGE_H
