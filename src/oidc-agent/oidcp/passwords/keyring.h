#ifndef OIDCAGENT_KEYRING_INTEGRATION_H
#define OIDCAGENT_KEYRING_INTEGRATION_H

#include "utils/oidc_error.h"

oidc_error_t keyring_savePasswordFor(const char* shortname,
                                     const char* password);
char*        keyring_getPasswordFor(const char* shortname);
oidc_error_t keyring_removePasswordFor(const char* shortname);

#endif  // OIDCAGENT_KEYRING_INTEGRATION_H
