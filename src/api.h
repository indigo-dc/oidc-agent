#ifndef OIDC_API_H
#define OIDC_API_H

#include "oidc_error.h"

#define OIDC_SOCK_ENV_NAME "OIDC_SOCK"

char* getAccessToken(const char* accountname, unsigned long min_valid_period) ;
char* getLoadedAccounts() ;
extern char* oidc_perror();
char* communicate(char* fmt, ...) ;

#endif // OIDC_API_H
