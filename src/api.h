#ifndef OIDC_API_H
#define OIDC_API_H

#include "oidc_error.h"

#define OIDC_SOCK_ENV_NAME "OIDC_SOCK"

char* getAccessToken(const char* providername, unsigned long min_valid_period) ;
char* getLoadedProvider() ;
extern char oidc_error[];
extern char* oidc_perror();
#endif // OIDC_API_H
