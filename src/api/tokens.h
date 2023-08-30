#ifndef OIDC_TOKEN_API_TOKENS_H
#define OIDC_TOKEN_API_TOKENS_H

#include <time.h>

#include "export_symbols.h"
#include "response.h"

/**
 * @brief gets a valid access token for an account config as well as related
 * information
 * @param accountname the short name of the account config for which an access
 * token should be returned
 * @param min_valid_period the minium period of time the access token has to be
 * valid in seconds
 * @param scope a space delimited list of scope values for the to be issued
 * access token. @c NULL if default value for that account configuration should
 * be used.
 * @param application_hint a hint indicating what application requests the
 * access token. This string might be displayed to the user.
 * @param audience Use this parameter to request an access token with this
 * specific audience. Can be a space separated list. @c NULL if no special
 * audience should be requested.
 * @return an agent_response struct containing the access token, issuer_url, and
 * expiration time in the @c token_response or an @c agent_error_response.
 * Has to be freed after usage using the @c secFreeAgentResponse function.
 */
LIB_PUBLIC struct agent_response getAgentTokenResponse(
    const char* accountname, time_t min_valid_period, const char* scope,
    const char* application_hint, const char* audience);

/**
 * @brief gets a valid access token for a specific provider as well as related
 * information
 * @param issuer_url the issuer url of the provider for which an access token
 * should be returned
 * @param min_valid_period the minium period of time the access token has to be
 * valid in seconds
 * @param scope a space delimited list of scope values for the to be issued
 * access token. @c NULL if default value for the used account configuration
 * should be used.
 * @param application_hint a hint indicating what application requests the
 * access token. This string might be displayed to the user.
 * @param audience Use this parameter to request an access token with this
 * specific audience. Can be a space separated list. @c NULL if no special
 * audience should be requested.
 * @return an agent_response struct containing the access token, issuer_url, and
 * expiration time in the @c token_response or an @c agent_error_response.
 * Has to be freed after usage using the @c secFreeAgentResponse function.
 */
LIB_PUBLIC struct agent_response getAgentTokenResponseForIssuer(
    const char* issuer_url, time_t min_valid_period, const char* scope,
    const char* application_hint, const char* audience);

/**
 * @brief gets a valid access token for an account config
 * @param accountname the short name of the account config for which an access
 * token should be returned
 * @param min_valid_period the minium period of time the access token has to be
 * valid in seconds
 * @param scope a space delimited list of scope values for the to be issued
 * access token. @c NULL if default value for the used account configuration
 * should be used.
 * @param application_hint a hint indicating what application requests the
 * access token. This string might be displayed to the user.
 * @param audience Use this parameter to request an access token with this
 * specific audience. Can be a space separated list. @c NULL if no special
 * audience should be requested.
 * @return a pointer to the access token. Has to be freed after usage using
 * @c secFree function. On failure @c NULL is returned and @c oidc_errno is set.
 */
LIB_PUBLIC char* getAccessToken(const char* accountname,
                                time_t min_valid_period, const char* scope,
                                const char* application_hint,
                                const char* audience);

/**
 * @brief gets a valid access token for an account config
 * @param issuer_url the issuer url of the provider for which an access token
 * should be returned
 * @param min_valid_period the minium period of time the access token has to be
 * valid in seconds
 * @param scope a space delimited list of scope values for the to be issued
 * access token. @c NULL if default value for that account configuration should
 * be used.
 * @param application_hint a hint indicating what application requests the
 * access token. This string might be displayed to the user.
 * @param audience Use this parameter to request an access token with this
 * specific audience. Can be a space separated list. @c NULL if no special
 * audience should be requested.
 * @return a pointer to the access token. Has to be freed after usage using
 * @c secFree function. On failure @c NULL is returned and @c oidc_errno is set.
 */
LIB_PUBLIC char* getAccessTokenForIssuer(const char* issuer_url,
                                         time_t      min_valid_period,
                                         const char* scope,
                                         const char* application_hint,
                                         const char* audience);

#endif  // OIDC_TOKEN_API_TOKENS_H
