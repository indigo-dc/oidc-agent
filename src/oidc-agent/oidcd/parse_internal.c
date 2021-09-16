#include "parse_internal.h"

#include "defines/ipc_values.h"
#include "utils/agentLogger.h"
#include "utils/json.h"
#include "utils/memory.h"
#include "utils/oidc_error.h"
#include "utils/printer.h"
#include "utils/string/stringUtils.h"

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

char* parseForInfo(char* res) {
  INIT_KEY_VALUE(IPC_KEY_INFO, OIDC_KEY_ERROR);
  if (CALL_GETJSONVALUES(res) < 0) {
    printError("Could not decode json: %s\n", res);
    printError("This seems to be a bug. Please hand in a bug report.\n");
    secFree(res);
    SEC_FREE_KEY_VALUES();
    return NULL;
  }
  secFree(res);
  KEY_VALUE_VARS(info, error);
  if (_error) {
    oidc_errno = OIDC_EERROR;
    oidc_seterror(_error);
    secFree(_info);
    secFree(_error);
    return NULL;
  }
  return _info;
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

char* parseStateLookupRes(char* res) {
  INIT_KEY_VALUE(IPC_KEY_STATUS, IPC_KEY_CONFIG, OIDC_KEY_ERROR);
  if (CALL_GETJSONVALUES(res) < 0) {
    secFree(res);
    return NULL;
  }
  KEY_VALUE_VARS(status, config, error);
  secFree(res);
  if (_error != NULL) {
    agent_log(ERROR, _error);
    SEC_FREE_KEY_VALUES();
    return NULL;
  }
  if (_config) {
    char* config = oidc_strcopy(_config);
    SEC_FREE_KEY_VALUES();
    return config;
  }
  if (strcaseequal(_status, STATUS_NOTFOUND)) {
    SEC_FREE_KEY_VALUES();
    oidc_errno = OIDC_EWRONGSTATE;
    return NULL;
  }
  SEC_FREE_KEY_VALUES();
  return NULL;
}
