#ifndef ISSUER_HELPER_H
#define ISSUER_HELPER_H

#include "account.h"

#include "../lib/list/src/list.h"

list_t* getSuggestableIssuers() ;
char* getFavIssuer(struct oidc_account* account, list_t* suggastable) ;
void printSuggestIssuer(list_t* suggastable) ;

#endif //ISSUER_HELPER_H

