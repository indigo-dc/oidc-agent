#ifndef OIDC_H
#define OIDC_H

#include <time.h>

int getAccessToken(int provider) ;
int tryRefreshFlow(int provider) ;
int tryPasswordFlow(int provider) ;
int refreshFlow(int povider_i) ;
int passwordFlow(int provider_i, const char* password) ;
int tokenIsValidForSeconds(int provider, time_t min_valid_period);

#endif //OIDC_H
