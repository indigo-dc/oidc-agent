#ifndef OIDC_H
#define OIDC_H

#include "account.h"
#include "oidc_error.h"

#include <time.h>

#define FORCE_NEW_TOKEN -1

oidc_error_t tryRefreshFlow(struct oidc_account* p) ;
oidc_error_t tryPasswordFlow(struct oidc_account* p) ;
oidc_error_t refreshFlow(struct oidc_account* p) ;
oidc_error_t passwordFlow(struct oidc_account* p) ;
int tokenIsValidForSeconds(struct oidc_account p, time_t min_valid_period);
char* dynamicRegistration(struct oidc_account* account, int useGrantType, const char* access_token) ;
oidc_error_t revokeToken(struct oidc_account* account) ;
oidc_error_t getIssuerConfig(struct oidc_account* account) ;
char* buildCodeFlowUri(struct oidc_account* account, char* state) ;
oidc_error_t codeExchange(struct oidc_account* account, const char* code, const char* used_redirect_uri) ;

#endif //OIDC_H
