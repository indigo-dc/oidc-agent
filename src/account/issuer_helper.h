#ifndef ISSUER_HELPER_H
#define ISSUER_HELPER_H

#include "account.h"

#include "list/list.h"

list_t* getSuggestableIssuers();
char*   getFavIssuer(struct oidc_account* account, list_t* suggastable);
void    printSuggestIssuer(list_t* suggastable);
void    printIssuerHelp(const char* url);
char*   getUsableResponseTypes(struct oidc_account account, list_t* flows);
char*   getUsableGrantTypes(struct oidc_account account, list_t* flows);

#endif  // ISSUER_HELPER_H
