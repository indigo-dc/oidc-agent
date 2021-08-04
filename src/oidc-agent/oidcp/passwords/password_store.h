#ifndef OIDC_PASSWORD_STORE_H
#define OIDC_PASSWORD_STORE_H

#include <time.h>

#include "utils/oidc_error.h"
#include "utils/password_entry.h"

oidc_error_t savePassword(struct password_entry* pw);
char*        getPasswordFor(const char* shortname);
oidc_error_t removePasswordFor(const char* shortname);
oidc_error_t removeAllPasswords();
void         removeDeathPasswords();
time_t       getMinPasswordDeath();

#endif  // OIDC_PASSWORD_STORE_H
