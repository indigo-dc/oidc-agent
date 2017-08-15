#ifndef OIDC_H
#define OIDC_H

#include <time.h>

#include "provider.h"

#define FORCE_NEW_TOKEN -1

int retrieveAccessToken(struct oidc_provider* p, time_t min_valid_period) ;
int tryRefreshFlow(struct oidc_provider* p) ;
int tryPasswordFlow(struct oidc_provider* p) ;
int refreshFlow(struct oidc_provider* p) ;
int passwordFlow(struct oidc_provider* p) ;
int tokenIsValidForSeconds(struct oidc_provider p, time_t min_valid_period);

#endif //OIDC_H
