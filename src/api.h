#ifndef OIDC_API_H
#define OIDC_API_H

char* getAccessToken(const char* accountname, unsigned long min_valid_period, const char* scope) ;
char* getLoadedAccounts() ;
extern void oidc_perror();
extern char* oidc_serror();
char* communicate(char* fmt, ...) ;

#endif // OIDC_API_H
