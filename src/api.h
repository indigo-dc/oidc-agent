#ifndef OIDC_API_H
#define OIDC_API_H

#include "ipc/ipc_values.h"
#include "oidc_error.h"

char* getAccessToken(const char* accountname, unsigned long min_valid_period, const char* scope) ;
char* getLoadedAccounts() ;
char* communicate(char* fmt, ...) ;
char* oidcagent_serror();
void oidcagent_perror();

#endif // OIDC_API_H
