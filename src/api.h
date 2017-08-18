#ifndef OIDC_API_H
#define OIDC_API_H

#define OIDC_SOCK_ENV_NAME "OIDC_SOCK"

char* getAccessToken(const char* providername, unsigned long min_valid_period) ;
char* getLoadedProvider() ;

#endif // OIDC_API_H
