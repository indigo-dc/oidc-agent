#include "parse.h"
#include "defines/agent_values.h"
#include "defines/ipc_values.h"
#include "defines/oidc_values.h"
#include "utils/json.h"
#include "utils/key_value.h"
#include "utils/printer.h"
#include "utils/stringUtils.h"
#include "wrapper/list.h"

struct loaded_accounts_response parseForLoadedAccountsListResponse(char* response) {
  if (response == NULL) {
    return (struct loaded_accounts_response){NULL};
  }

  INIT_KEY_VALUE(IPC_KEY_STATUS, OIDC_KEY_ERROR, "info");
  if (CALL_GETJSONVALUES(response) < 0) {
    printError("Read malformed data. Please hand in bug report.\n");
    secFree(response);
    SEC_FREE_KEY_VALUES();
    return (struct loaded_accounts_response){NULL};
  }

  secFree(response);
  KEY_VALUE_VARS(status, error, info);
  if (_error) {  // error
    oidc_errno = OIDC_EERROR;
    oidc_seterror(_error);
    SEC_FREE_KEY_VALUES();
    return (struct loaded_accounts_response){NULL};
  } else {
    secFree(_status);
    oidc_errno        = OIDC_SUCCESS;
    char *accounts = JSONArrayStringToDelimitedString(_info, "||");
    return (struct loaded_accounts_response){accounts};
  }
}
