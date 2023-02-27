#ifndef OIDCGEN_PROMPTANDSET_H
#define OIDCGEN_PROMPTANDSET_H

#include "account/account.h"
#include "name.h"
#include "oidc-gen/oidc-gen_options.h"

int  readClientId(struct oidc_account*, const struct arguments*);
void askClientId(struct oidc_account*, const struct arguments*);
void needClientId(struct oidc_account*, const struct arguments*);
void askOrNeedClientId(struct oidc_account*, const struct arguments*, int);

int  readClientSecret(struct oidc_account*, const struct arguments*);
void askClientSecret(struct oidc_account*, const struct arguments*);
void needClientSecret(struct oidc_account*, const struct arguments*);
void askOrNeedClientSecret(struct oidc_account*, const struct arguments*, int);

void _suggestTheseIssuers(list_t* issuers, struct oidc_account* account,
                          const struct arguments* arguments, int optional);
int  readIssuer(struct oidc_account*, const struct arguments*);
void askIssuer(struct oidc_account*, const struct arguments*);
void needIssuer(struct oidc_account*, const struct arguments*);
void askOrNeedIssuer(struct oidc_account*, const struct arguments*, int);

int  readMytokenIssuer(struct oidc_account*, const struct arguments*);
void askMytokenIssuer(struct oidc_account*, const struct arguments*);
void needMytokenIssuer(struct oidc_account*, const struct arguments*);
void askOrNeedMytokenIssuer(struct oidc_account*, const struct arguments*, int);

int  readMyProfile(struct oidc_account*, const struct arguments*);
void askMyProfile(struct oidc_account*, const struct arguments*);
void needMyProfile(struct oidc_account*, const struct arguments*);
void askOrNeedMyProfile(struct oidc_account*, const struct arguments*, int);

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

void _askOrNeedScope(char* supportedScope, struct oidc_account* account,
                     const struct arguments* arguments, int optional);
int  readScope(struct oidc_account*, const struct arguments*);
void askScope(struct oidc_account*, const struct arguments*);
void needScope(struct oidc_account*, const struct arguments*);
void askOrNeedScope(struct oidc_account*, const struct arguments*, int);

int  readRedirectUris(struct oidc_account*, const struct arguments*);
void askRedirectUris(struct oidc_account*, const struct arguments*);
void needRedirectUris(struct oidc_account*, const struct arguments*);
void askOrNeedRedirectUris(struct oidc_account*, const struct arguments*, int);

int  readDeviceAuthEndpoint(struct oidc_account*, const struct arguments*);
void askDeviceAuthEndpoint(struct oidc_account*, const struct arguments*);
void needDeviceAuthEndpoint(struct oidc_account*, const struct arguments*);
void askOrNeedDeviceAuthEndpoint(struct oidc_account*, const struct arguments*,
                                 int);

int  readConfigEndpoint(struct oidc_account*, const struct arguments*);
void askConfigEndpoint(struct oidc_account*, const struct arguments*);
void needConfigEndpoint(struct oidc_account*, const struct arguments*);
void askOrNeedConfigEndpoint(struct oidc_account*, const struct arguments*,
                             int);

#endif  // OIDCGEN_PROMPTANDSET_H
