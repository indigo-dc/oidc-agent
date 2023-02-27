#ifndef OIDCGEN_PROMPTANDSET_NAME_H
#define OIDCGEN_PROMPTANDSET_NAME_H

#include "account/account.h"

int  readName(struct oidc_account*, const char*, const char*);
void askName(struct oidc_account*, unsigned char, const char*, const char*);
void needName(struct oidc_account*, unsigned char, const char*, const char*);
void askOrNeedName(struct oidc_account*, const char*, const char*,
                   unsigned char, unsigned char, const char*);

#endif  // OIDCGEN_PROMPTANDSET_NAME_H
