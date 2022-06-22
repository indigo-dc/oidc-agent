#ifndef OIDC_ADD_API_H
#define OIDC_ADD_API_H

#include "export_symbols.h"
#include "response.h"

/**
 * @brief gets a list of loaded accounts
 * @return a pointer to string with loaded accounts. Has to be freed after usage
 * using
 * @c secFree function. On failure @c NULL is returned and @c oidc_errno is set.
 */
LIB_PUBLIC char* getLoadedAccountsList();

/**
 * @brief gets a list of loaded accounts
 * @return an agent_response struct containing a string list of the loaded
 * accounts. Has to be freed after usage using the @c secFreeAgentResponse
 * function. On failure @c NULL is returned and @c oidc_errno is set.
 */
LIB_PUBLIC struct agent_response getAgentLoadedAccountsListResponse();

#endif  // OIDC_ADD_API_H
