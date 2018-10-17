#ifndef OIDC_API_H
#define OIDC_API_H

/**
 * @struct token_response api.h
 * @brief a struct holding an access token and the associated issuer
 */
struct token_response {
  char* token;
  char* issuer;
};

/** @fn char* getAccessToken(const char* accountname, unsigned long
 * min_valid_period, const char* scope
 * @brief gets a valid access token for an account config
 * @deprecated use getTokenResponse instead to additionally get the issuer_url
 * for the returned access token
 * @param accountname the short name of the account config for which an access
 * token should be returned
 * @param min_valid_period the minium period of time the access token has to be
 * valid in seconds
 * @param scope a space delimited list of scope values for the to be issued
 * access token. NULL if default value for that account configuration should be
 * used.
 * @return a pointer to the access token. Has to be freed after usage. On
 * failure NULL is returned and oidc_errno is set.
 */
char* getAccessToken(const char* accountname, unsigned long min_valid_period,
                     const char* scope);

/** @fn struct token_response getTokenResponse(const char* accountname, unsigned
 * long min_valid_period, const char* scope)
 * @brief gets a valid access token for an account config
 * @param accountname the short name of the account config for which an access
 * token should be returned
 * @param min_valid_period the minium period of time the access token has to be
 * valid in seconds
 * @param scope a space delimited list of scope values for the to be issued
 * access token. NULL if default value for that account configuration should be
 * used.
 * @return a token_response struct contain the access token and the issuer_url.
 * Has to be freed after usage using the secFreeTokenResponse function. On
 * failure an struct with two NULL pointers is returned and oidc_errno is set.
 */
struct token_response getTokenResponse(const char*   accountname,
                                       unsigned long min_valid_period,
                                       const char*   scope);

/** @fn char* getLoadedAccount()
 * @brief gets a list of currently loaded accounts
 * @return a pointer to the JSON Array String containing all the short names
 * of the currently loaded accounts. Has to be freed after usage.
 * On failure NULL is returned and oidc_errno is set.
 */
char* getLoadedAccounts();

/**
 * @brief gets an error string detailing the last occured error
 * @return the error string. MUST NOT be freed.
 */
char* oidcagent_serror();

/**
 * @brief prints an error message to stderr detailing the last occured error
 */
void oidcagent_perror();

/**
 * @brief clears and frees a token_response struct
 * @param token_response the struct to be freed
 */
void secFreeTokenResponse(struct token_response token_response);

extern void _secFree(void* ptr);

#ifndef secFree
#define secFree(ptr) \
  do {               \
    _secFree((ptr)); \
    (ptr) = NULL;    \
  } while (0)
#endif  // secFree

#endif  // OIDC_API_H
