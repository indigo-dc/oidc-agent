#ifndef OIDC_ADD_API_H
#define OIDC_ADD_API_H

#include "api/export_symbols.h"

#include <time.h>

/**
 * @struct loaded_accounts_response oidc-add/api.h
 * @brief a struct holding loaded accounts list as a string with delimiters
 */
LIB_PUBLIC struct loaded_accounts_response {
  char* accounts;
};

/**
 * @brief gets a list of loaded accounts
 * @return a pointer to string with laoded accounts. Has to be freed after usage using
 * @c secFree function. On failure @c NULL is returned and @c oidc_errno is set.
 */
LIB_PUBLIC char* getLoadedAccountsList();


/**
 * @brief clears and frees a loaded_accounts_response struct
 * @param loaded_accounts_response the struct to be freed
 */
LIB_PUBLIC void secFreeLoadedAccountsListResponse(struct loaded_accounts_response token_response);


#endif  // OIDC_ADD_API_H
