#ifndef OIDCGEN_PROMPTANDSET_H
#define OIDCGEN_PROMPTANDSET_H

#include "account/account.h"
#include "oidc-gen/oidc-gen_options.h"

int  readClientId(struct oidc_account*, const struct arguments*);
void askClientId(struct oidc_account*, const struct arguments*);
void needClientId(struct oidc_account*, const struct arguments*);
void askOrNeedClientId(struct oidc_account*, const struct arguments*, int);

int  readClientSecret(struct oidc_account*, const struct arguments*);
void askClientSecret(struct oidc_account*, const struct arguments*);
void needClientSecret(struct oidc_account*, const struct arguments*);
void askOrNeedClientSecret(struct oidc_account*, const struct arguments*, int);

int  readIssuer(struct oidc_account*, const struct arguments*);
void askIssuer(struct oidc_account*, const struct arguments*);
void needIssuer(struct oidc_account*, const struct arguments*);

int  readRefreshToken(struct oidc_account*, const struct arguments*);
void askRefreshToken(struct oidc_account*, const struct arguments*);
void needRefreshToken(struct oidc_account*, const struct arguments*);
void askOrNeedRefreshToken(struct oidc_account*, const struct arguments*, int);

int  readAudience(struct oidc_account*, const struct arguments*);
void askAudience(struct oidc_account*, const struct arguments*);
void needAudience(struct oidc_account*, const struct arguments*);
void askOrNeedAudience(struct oidc_account*, const struct arguments*, int);

int  readUsername(struct oidc_account*, const struct arguments*);
void askUsername(struct oidc_account*, const struct arguments*);
void needUsername(struct oidc_account*, const struct arguments*);
void askOrNeedUsername(struct oidc_account*, const struct arguments*, int);

int  readPassword(struct oidc_account*, const struct arguments*);
void askPassword(struct oidc_account*, const struct arguments*);
void needPassword(struct oidc_account*, const struct arguments*);
void askOrNeedPassword(struct oidc_account*, const struct arguments*, int);

int  readCertPath(struct oidc_account*, const struct arguments*);
void askCertPath(struct oidc_account*, const struct arguments*);
void needCertPath(struct oidc_account*, const struct arguments*);
void askOrNeedCertPath(struct oidc_account*, const struct arguments*, int);

int  readName(struct oidc_account*, const struct arguments*);
void askName(struct oidc_account*, const struct arguments*);
void needName(struct oidc_account*, const struct arguments*);
void askOrNeedName(struct oidc_account*, const struct arguments*, int);

int  readScope(struct oidc_account*, const struct arguments*);
void askScope(struct oidc_account*, const struct arguments*);
void needScope(struct oidc_account*, const struct arguments*);
void askOrNeedScope(struct oidc_account*, const struct arguments*, int);

int  readRedirectUris(struct oidc_account*, const struct arguments*);
void askRedirectUris(struct oidc_account*, const struct arguments*);
void needRedirectUris(struct oidc_account*, const struct arguments*);
void askOrNeedRedirectUris(struct oidc_account*, const struct arguments*, int);

#endif  // OIDCGEN_PROMPTANDSET_H
