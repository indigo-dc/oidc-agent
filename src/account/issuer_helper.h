#ifndef ISSUER_HELPER_H
#define ISSUER_HELPER_H

#include "account.h"

#include "wrapper/list.h"

list_t* getSuggestableIssuers();
size_t  getFavIssuer(const struct oidc_account* account, list_t* suggestable);
void    printSuggestIssuer(list_t* suggastable);
void    printIssuerHelp(const char* url);
char* getUsableResponseTypes(const struct oidc_account* account, list_t* flows);
char* getUsableGrantTypes(const struct oidc_account* account, list_t* flows);
int   compIssuerUrls(const char* a, const char* b);

#endif  // ISSUER_HELPER_H
