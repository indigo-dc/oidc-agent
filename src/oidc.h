#ifndef OIDC_H
#define OIDC_H

#include <time.h>

#include "provider.h"
#include "oidc_error.h"

#define FORCE_NEW_TOKEN -1

oidc_error_t retrieveAccessToken(struct oidc_provider* p, time_t min_valid_period) ;
oidc_error_t retrieveAccessTokenRefreshFlowOnly(struct oidc_provider* p, time_t min_valid_period) ;
oidc_error_t tryRefreshFlow(struct oidc_provider* p) ;
oidc_error_t tryPasswordFlow(struct oidc_provider* p) ;
oidc_error_t refreshFlow(struct oidc_provider* p) ;
oidc_error_t passwordFlow(struct oidc_provider* p) ;
int tokenIsValidForSeconds(struct oidc_provider p, time_t min_valid_period);
char* dynamicRegistration(struct oidc_provider* provider, int useGrantType) ;
oidc_error_t getEndpoints(struct oidc_provider* provider) ;

#endif //OIDC_H
