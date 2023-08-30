#ifndef ACCOUNT_SETANDGET_H
#define ACCOUNT_SETANDGET_H

#include "account/account.h"

struct oidc_issuer* account_getIssuer(const struct oidc_account* p);
char*               account_getIssuerUrl(const struct oidc_account* p);
char*               account_getMytokenUrl(const struct oidc_account* p);
char*               account_getConfigEndpoint(const struct oidc_account* p);
char*               account_getTokenEndpoint(const struct oidc_account* p);
char* account_getAuthorizationEndpoint(const struct oidc_account* p);
char* account_getRevocationEndpoint(const struct oidc_account* p);
char* account_getRegistrationEndpoint(const struct oidc_account* p);
char* account_getDeviceAuthorizationEndpoint(const struct oidc_account* p);
char* account_getMytokenEndpoint(const struct oidc_account* p);
char* account_getScopesSupported(const struct oidc_account* p);
char* account_getGrantTypesSupported(const struct oidc_account* p);
char* account_getResponseTypesSupported(const struct oidc_account* p);
char* account_getName(const struct oidc_account* p);
char* account_getClientName(const struct oidc_account* p);
char* account_getClientId(const struct oidc_account* p);
char* account_getClientSecret(const struct oidc_account* p);
char* account_getScope(const struct oidc_account* p);
char* account_getAudience(const struct oidc_account* p);
char* account_getUsedMytokenProfile(const struct oidc_account* p);
char* account_getUsername(const struct oidc_account* p);
char* account_getPassword(const struct oidc_account* p);
char* account_getRefreshToken(const struct oidc_account* p);
char* account_getAccessToken(const struct oidc_account* p);
unsigned long account_getTokenExpiresAt(const struct oidc_account* p);
char*         account_getCertPath(const struct oidc_account* p);
char*         account_getCertPathOrDefault(const struct oidc_account* p);
list_t*       account_getRedirectUris(const struct oidc_account* p);
size_t        account_getRedirectUrisCount(const struct oidc_account* p);
char*         account_getUsedState(const struct oidc_account* p);
time_t        account_getDeath(const struct oidc_account* p);
char*         account_getCodeChallengeMethod(const struct oidc_account* p);
unsigned char account_getConfirmationRequired(const struct oidc_account* p);
unsigned char account_getNoWebServer(const struct oidc_account* p);
unsigned char account_getNoScheme(const struct oidc_account* p);
unsigned char account_getAlwaysAllowId(const struct oidc_account* p);
unsigned char account_getIsOAuth2(const struct oidc_account* p);
unsigned char account_getUsesPubClient(const struct oidc_account* p);

void account_setIssuerUrl(struct oidc_account* p, char* issuer_url);
void account_setMytokenUrl(struct oidc_account* p, char* issuer_url);
void account_setConfigEndpoint(struct oidc_account* p, char* config_endpoint);
void account_setClientName(struct oidc_account* p, char* clientname);
void account_setName(struct oidc_account* p, char* shortname,
                     const char* client_identifier);
void account_setClientId(struct oidc_account* p, char* client_id);
void account_setClientSecret(struct oidc_account* p, char* client_secret);
void account_setScopeExact(struct oidc_account* p, char* scope);
void account_setScope(struct oidc_account* p, char* scope);
void account_setIssuer(struct oidc_account* p, struct oidc_issuer* issuer);
void account_setScopesSupported(struct oidc_account* p, char* scopes_supported);
void account_setAudience(struct oidc_account* p, char* audience);
void account_setUsedMytokenProfile(struct oidc_account* p, char* profile);
void account_setUsername(struct oidc_account* p, char* username);
void account_setPassword(struct oidc_account* p, char* password);
void account_setRefreshToken(struct oidc_account* p, char* refresh_token);
void account_setAccessToken(struct oidc_account* p, char* access_token);
void account_setTokenExpiresAt(struct oidc_account* p,
                               unsigned long        token_expires_at);
void account_setCertPath(struct oidc_account* p, char* cert_path);
void account_setRedirectUris(struct oidc_account* p, list_t* redirect_uris);
void account_setUsedState(struct oidc_account* p, char* used_state);
void account_clearCredentials(struct oidc_account* a);
void account_setDeath(struct oidc_account* p, time_t death);
void account_setCodeChallengeMethod(struct oidc_account* p,
                                    char*                code_challenge_method);
void account_setConfirmationRequired(struct oidc_account* p);
void account_setNoWebServer(struct oidc_account* p);
void account_setNoScheme(struct oidc_account* p);
void account_setAlwaysAllowId(struct oidc_account* p);
void account_setOAuth2(struct oidc_account* p);
void account_setUsesPubClient(struct oidc_account* p);

int account_refreshTokenIsValid(const struct oidc_account* p);

#endif  // ACCOUNT_SETANDGET_H
