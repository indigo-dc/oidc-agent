#ifndef OIDC_API_H
#define OIDC_API_H

#include <time.h>

#include "export_symbols.h"

/**
 * @struct token_response api.h
 * @brief a struct holding an access token, the associated issuer, and the
 * expiration time of the token
 */
LIB_PUBLIC struct token_response {
  char*  token;
  char*  issuer;
  time_t expires_at;
};

/**
 * @struct agent_error_response api.h
 * @brief a struct holding an error message and optionally a help message
 */
LIB_PUBLIC struct agent_error_response {
  char* error;
  char* help;
};

#define AGENT_RESPONSE_TYPE_ERROR 0
#define AGENT_RESPONSE_TYPE_TOKEN 1

/**
 * @struct agent_response api.h
 * @brief a struct holding either a @c token_response or @c agent_error_response
 */
LIB_PUBLIC struct agent_response {
  unsigned char type;
  union {
    struct token_response       token_response;
    struct agent_error_response error_response;
  };
};

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
 * @deprecated use @c getTokenResponse3 instead to additionally get the
 * issuer_url and expiration date for the returned access token or if only the
 * access token is required @c getAccessToken3
 * @param accountname the short name of the account config for which an access
 * token should be returned
 * @param min_valid_period the minium period of time the access token has to be
 * valid in seconds
 * @param scope a space delimited list of scope values for the to be issued
 * access token. @c NULL if default value for that account configuration should
 * be used.
 * @return a pointer to the access token. Has to be freed after usage using
 * @c secFree function. On failure @c NULL is returned and @c oidc_errno is set.
 */
LIB_PUBLIC char* getAccessToken(const char* accountname,
                                time_t min_valid_period, const char* scope);

/**
 * @brief gets a valid access token for an account config
 * @deprecated use @c getAccessToken3 or @c getTokenResponse3 instead
 * @param issuer_url the issuer url of the provider for which an access token
 * should be returned
 * @param min_valid_period the minium period of time the access token has to be
 * valid in seconds
 * @param scope a space delimited list of scope values for the to be issued
 * access token. @c NULL if default value for the used account configuration
 * should be used.
 * @param application_hint a hint indicating what application requests the
 * access token. This string might be displayed to the user.
 * @return a pointer to the access token. Has to be freed after usage using
 * @c secFree function. On failure @c NULL is returned and @c oidc_errno is set.
 */
LIB_PUBLIC char* getAccessToken2(const char* accountname,
                                 time_t min_valid_period, const char* scope,
                                 const char* application_hint);

/**
 * @brief gets a valid access token for an account config
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
 * @return a pointer to the access token. Has to be freed after usage using
 * @c secFree function. On failure @c NULL is returned and @c oidc_errno is set.
 */
LIB_PUBLIC char* getAccessToken3(const char* accountname,
                                 time_t min_valid_period, const char* scope,
                                 const char* application_hint,
                                 const char* audience);

/**
 * @brief gets a valid access token for an account config
 * @deprecated use @c getAccessTokenForIssuer3 or @c getTokenResponseForIssuer3
 * instead
 * @param accountname the short name of the account config for which an access
 * token should be returned
 * @param min_valid_period the minium period of time the access token has to be
 * valid in seconds
 * @param scope a space delimited list of scope values for the to be issued
 * access token. @c NULL if default value for that account configuration should
 * be used.
 * @param application_hint a hint indicating what application requests the
 * access token. This string might be displayed to the user.
 * @return a pointer to the access token. Has to be freed after usage using
 * @c secFree function. On failure @c NULL is returned and @c oidc_errno is set.
 */
LIB_PUBLIC char* getAccessTokenForIssuer(const char* issuer_url,
                                         time_t      min_valid_period,
                                         const char* scope,
                                         const char* application_hint);

/**
 * @brief gets a valid access token for an account config
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
 * @return a pointer to the access token. Has to be freed after usage using
 * @c secFree function. On failure @c NULL is returned and @c oidc_errno is set.
 */
LIB_PUBLIC char* getAccessTokenForIssuer3(const char* issuer_url,
                                          time_t      min_valid_period,
                                          const char* scope,
                                          const char* application_hint,
                                          const char* audience);

/**
 * @brief gets a valid access token for an account config as well as related
 * information
 * @deprecated use @c getTokenResponse3 instead
 * @param accountname the short name of the account config for which an access
 * token should be returned
 * @param min_valid_period the minium period of time the access token has to be
 * valid in seconds
 * @param scope a space delimited list of scope values for the to be issued
 * access token. @c NULL if default value for that account configuration should
 * be used.
 * @param application_hint a hint indicating what application requests the
 * access token. This string might be displayed to the user.
 * @return a token_response struct containing the access token, issuer_url, and
 * expiration time.
 * Has to be freed after usage using the @c secFreeTokenResponse function. On
 * failure a zeroed struct is returned and @c oidc_errno is set.
 */
LIB_PUBLIC struct token_response getTokenResponse(const char* accountname,
                                                  time_t      min_valid_period,
                                                  const char* scope,
                                                  const char* application_hint);

/**
 * @brief gets a valid access token for an account config as well as related
 * information
 * @deprecated use @c getAgentTokenResponse instead to additionally get an error
 * and help message on failure
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
 * @return a token_response struct containing the access token, issuer_url, and
 * expiration time.
 * Has to be freed after usage using the @c secFreeTokenResponse function. On
 * failure a zeroed struct is returned and @c oidc_errno is set.
 */
LIB_PUBLIC struct token_response getTokenResponse3(const char* accountname,
                                                   time_t      min_valid_period,
                                                   const char* scope,
                                                   const char* application_hint,
                                                   const char* audience);

/**
 * @brief gets a valid access token for a specific provider as well as related
 * information
 * @deprecated use @c getTokenResponseForIssuer3 instead
 * @param issuer_url the issuer url of the provider for which an access token
 * should be returned
 * @param min_valid_period the minium period of time the access token has to be
 * valid in seconds
 * @param scope a space delimited list of scope values for the to be issued
 * access token. @c NULL if default value for the used account configuration
 * should be used.
 * @param application_hint a hint indicating what application requests the
 * access token. This string might be displayed to the user.
 * @return a token_response struct containing the access token, issuer_url, and
 * expiration time.
 * Has to be freed after usage using the @c secFreeTokenResponse function. On
 * failure a zeroed struct is returned and @c oidc_errno is set.
 */
LIB_PUBLIC struct token_response getTokenResponseForIssuer(
    const char* issuer_url, time_t min_valid_period, const char* scope,
    const char* application_hint);

/**
 * @brief gets a valid access token for a specific provider as well as related
 * information
 * @deprecated use @c getAgentTokenResponseForIssuer instead to additionally get
 * an error and help message on failure
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
 * @return a token_response struct containing the access token, issuer_url, and
 * expiration time.
 * Has to be freed after usage using the @c secFreeTokenResponse function. On
 * failure a zeroed struct is returned and @c oidc_errno is set.
 */
LIB_PUBLIC struct token_response getTokenResponseForIssuer3(
    const char* issuer_url, time_t min_valid_period, const char* scope,
    const char* application_hint, const char* audience);

/**
 * @brief gets an error string detailing the last occurred error
 * @return the error string. MUST NOT be freed.
 */
LIB_PUBLIC char* oidcagent_serror();

/**
 * @brief prints an error message to stderr detailing the last occurred error
 * @note Since version 4.2.0 you most likely want to use @c
 * oidcagent_printErrorResponse instead
 */
LIB_PUBLIC void oidcagent_perror();

/**
 * @brief prints the error and help messages (if set) of the passed @c
 * agent_error_response struct to @c stderr
 */
LIB_PUBLIC void oidcagent_printErrorResponse(struct agent_error_response err);

/**
 * @brief clears and frees a token_response struct
 * @param token_response the struct to be freed
 */
LIB_PUBLIC void secFreeTokenResponse(struct token_response token_response);

/**
 * @brief clears and frees an agent_error_response struct
 * @param error_response the struct to be freed
 */
LIB_PUBLIC void secFreeErrorResponse(
    struct agent_error_response error_response);

/**
 * @brief clears and frees an agent_response struct
 * @param agent_response the struct to be freed
 */
LIB_PUBLIC void secFreeAgentResponse(struct agent_response agent_response);

extern LIB_PUBLIC void _secFree(void*);

#ifndef secFree
#define secFree(ptr) \
  do {               \
    _secFree((ptr)); \
    (ptr) = NULL;    \
  } while (0)
#endif  // secFree

#endif  // OIDC_API_H
