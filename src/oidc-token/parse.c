#include "parse.h"

#include "defines/agent_values.h"
#include "defines/ipc_values.h"
#include "defines/oidc_values.h"
#include "utils/json.h"
#include "utils/key_value.h"
#include "utils/printer.h"
#include "utils/stringUtils.h"

struct token_response parseForTokenResponse(char* response) {
  if (response == NULL) {
    return (struct token_response){NULL, NULL, 0};
  }
  INIT_KEY_VALUE(IPC_KEY_STATUS, OIDC_KEY_ERROR, OIDC_KEY_ACCESSTOKEN,
                 OIDC_KEY_ISSUER, AGENT_KEY_EXPIRESAT);
  if (CALL_GETJSONVALUES(response) < 0) {
    printError("Read malformed data. Please hand in bug report.\n");
    secFree(response);
    SEC_FREE_KEY_VALUES();
    return (struct token_response){NULL, NULL, 0};
  }
  secFree(response);
  KEY_VALUE_VARS(status, error, access_token, issuer, expires_at);
  if (_error) {  // error
    oidc_errno = OIDC_EERROR;
    oidc_seterror(_error);
    SEC_FREE_KEY_VALUES();
    return (struct token_response){NULL, NULL, 0};
  } else {
    secFree(_status);
    oidc_errno        = OIDC_SUCCESS;
    time_t expires_at = strToULong(_expires_at);
    secFree(_expires_at);
    return (struct token_response){_access_token, _issuer, expires_at};
  }
}
