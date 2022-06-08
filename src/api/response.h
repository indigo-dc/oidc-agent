#ifndef OIDC_AGENT_RESPONSE_H
#define OIDC_AGENT_RESPONSE_H

#include <time.h>

#include "export_symbols.h"

/**
 * @struct token_response response.h
 * @brief a struct holding an access token, the associated issuer, and the
 * expiration time of the token
 */
LIB_PUBLIC struct token_response {
  char*  token;
  char*  issuer;
  time_t expires_at;
};

/**
 * @struct agent_error_response response.h
 * @brief a struct holding an error message and optionally a help message
 */
LIB_PUBLIC struct agent_error_response {
  char* error;
  char* help;
};

/**
 * @struct loaded_accounts_response oidc-add/api.h
 * @brief a struct holding loaded accounts list as a string with delimiters
 */
LIB_PUBLIC struct loaded_accounts_response { char* accounts; };

/**
 * @struct mytoken_response response.h
 * @brief a struct holding a mytoken, the associated mytoken issuer, OIDC
 * issuer, and other information about the mytoken
 */
LIB_PUBLIC struct mytoken_response {
  char*  token;
  char*  token_type;  // As returned from the mytoken server
  char*  mytoken_issuer;
  char*  oidc_issuer;
  char*  restrictions;           // String holding the JSON
  char*  capabilities;           // String holding the JSON
  char*  subtoken_capabilities;  // String holding the JSON
  char*  rotation;               // String holding the JSON
  time_t expires_at;
};

#define AGENT_RESPONSE_TYPE_ERROR 0
#define AGENT_RESPONSE_TYPE_TOKEN 1
#define AGENT_RESPONSE_TYPE_ACCOUNTS 2
#define AGENT_RESPONSE_TYPE_MYTOKEN 3

/**
 * @struct agent_response response.h
 * @brief a struct holding either a @c token_response or @c agent_error_response
 */
LIB_PUBLIC struct agent_response {
  unsigned char type;
  union {
    struct token_response           token_response;
    struct agent_error_response     error_response;
    struct loaded_accounts_response loaded_accounts_response;
    struct mytoken_response         mytoken_response;
  };
};

/**
 * @brief clears and frees a token_response struct
 * @param token_response the struct to be freed
 */
LIB_PUBLIC void secFreeTokenResponse(struct token_response);

/**
 * @brief clears and frees a mytoken_response struct
 * @param mytoken_response the struct to be freed
 */
LIB_PUBLIC void secFreeMytokenResponse(struct mytoken_response);

/**
 * @brief clears and frees an agent_error_response struct
 * @param error_response the struct to be freed
 */
LIB_PUBLIC void secFreeErrorResponse(struct agent_error_response);

/**
 * @brief clears and frees a loaded_accounts_response struct
 * @param loaded_accounts_response the struct to be freed
 */
LIB_PUBLIC void secFreeLoadedAccountsListResponse(
    struct loaded_accounts_response);

/**
 * @brief clears and frees an agent_response struct
 * @param agent_response the struct to be freed
 */
LIB_PUBLIC void secFreeAgentResponse(struct agent_response);

unsigned char _checkLocalResponseForRemote(struct agent_response);

/**
 * @brief prints the error and help messages (if set) of the passed @c
 * agent_error_response struct to @c stderr
 */
LIB_PUBLIC void oidcagent_printErrorResponse(struct agent_error_response err);

#endif  // OIDC_AGENT_RESPONSE_H
