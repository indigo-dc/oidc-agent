#ifndef OIDC_TOKEN_API_MYTOKENS_H
#define OIDC_TOKEN_API_MYTOKENS_H

#include "export_symbols.h"
#include "response.h"

/**
 * @brief gets a new mytoken for a mytoken account config as well as related
 * information
 * @param accountname the short name of the account config to be used for
 * obtaining a mytoken
 * @param mytoken_profile the mytoken profile that specifies the desired
 * properties for the requested mytoken
 * @param application_hint a hint indicating what application requests the
 * access token. This string might be displayed to the user.
 * @return an agent_response struct containing the mytoken and additional
 * information in the @c mytoken_response or an @c agent_error_response. Has to
 * be freed after usage using the @c secFreeAgentResponse function.
 */
LIB_PUBLIC struct agent_response getAgentMytokenResponse(
    const char* accountname, const char* mytoken_profile,
    const char* application_hint);

/**
 * @brief gets a new mytoken for a mytoken account config
 * @param accountname the short name of the account config to be used for
 * obtaining a mytoken
 * @param mytoken_profile the mytoken profile that specifies the desired
 * properties for the requested mytoken
 * @param application_hint a hint indicating what application requests the
 * access token. This string might be displayed to the user.
 * @return a pointer to the mytoken. Has to be freed after usage using
 * @c secFree function. On failure @c NULL is returned and @c oidc_errno is set.
 */
LIB_PUBLIC char* getMytoken(const char* accountname,
                            const char* mytoken_profile,
                            const char* application_hint);

#endif  // OIDC_TOKEN_API_MYTOKENS_H
