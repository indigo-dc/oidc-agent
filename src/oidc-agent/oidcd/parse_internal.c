#include "parse_internal.h"
#include "defines/ipc_values.h"
#include "utils/json.h"
#include "utils/memory.h"
#include "utils/oidc_error.h"

char* parseForConfig(char* res) {
  INIT_KEY_VALUE(INT_IPC_KEY_OIDCERRNO, IPC_KEY_CONFIG);
  if (CALL_GETJSONVALUES(res) < 0) {
    printError("Could not decode json: %s\n", res);
    printError("This seems to be a bug. Please hand in a bug report.\n");
    secFree(res);
    SEC_FREE_KEY_VALUES();
    return NULL;
  }
  secFree(res);
  KEY_VALUE_VARS(oidc_errno, config);
  if (_oidc_errno) {
    oidc_errno = strToInt(_oidc_errno);
    secFree(_oidc_errno);
  }
  return _config;
}

oidc_error_t parseForErrorCode(char* res) {
  INIT_KEY_VALUE(INT_IPC_KEY_OIDCERRNO);
  if (CALL_GETJSONVALUES(res) < 0) {
    printError("Could not decode json: %s\n", res);
    printError("This seems to be a bug. Please hand in a bug report.\n");
    secFree(res);
    return oidc_errno;
  }
  secFree(res);
  KEY_VALUE_VARS(oidc_errno);
  if (_oidc_errno) {
    oidc_errno = strToInt(_oidc_errno);
    secFree(_oidc_errno);
    return oidc_errno;
  }
  return OIDC_SUCCESS;
}
