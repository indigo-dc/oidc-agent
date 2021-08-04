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
 */
LIB_PUBLIC void oidcagent_perror();

/**
 * @brief clears and frees a token_response struct
 * @param token_response the struct to be freed
 */
LIB_PUBLIC void secFreeTokenResponse(struct token_response token_response);

extern LIB_PUBLIC void _secFree(void*);

#ifndef secFree
#define secFree(ptr) \
  do {               \
    _secFree((ptr)); \
    (ptr) = NULL;    \
  } while (0)
#endif  // secFree

#endif  // OIDC_API_H
