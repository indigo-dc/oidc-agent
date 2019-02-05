#ifndef OIDC_PASSWORD_STORE_H
#define OIDC_PASSWORD_STORE_H

#include "password_entry.h"
#include "utils/oidc_error.h"

oidc_error_t           savePassword(struct password_entry* pw);
char*                  getPasswordFor(const char* shortname);
oidc_error_t           removePasswordFor(const char* shortname);
struct password_entry* getDeathPasswordEntry();

#endif  // OIDC_PASSWORD_STORE_H
