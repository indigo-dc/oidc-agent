#ifndef OIDC_PASSWORD_STORE_H
#define OIDC_PASSWORD_STORE_H

#include "password_entry.h"
#include "utils/oidc_error.h"

#include <time.h>

oidc_error_t savePassword(struct password_entry* pw);
char*        getPasswordFor(const char* shortname);
oidc_error_t removePasswordFor(const char* shortname);
oidc_error_t removeAllPasswords();
void         removeDeathPasswords();
time_t       getMinPasswordDeath();

#endif  // OIDC_PASSWORD_STORE_H
