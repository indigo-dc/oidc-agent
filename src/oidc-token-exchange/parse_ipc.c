#include "parse_ipc.h"
#include "defines/ipc_values.h"
#include "utils/json.h"
#include "utils/key_value.h"
#include "utils/pass.h"

/**
 * @param res a pointer to the response that should be parsed. The pointer will
 * be freed!
 * @param arguments the arguments oidc-token-exchange was called with
 */
char* exchange_parseResponse(char* res, const struct arguments* arguments) {
  INIT_KEY_VALUE(IPC_KEY_STATUS, IPC_KEY_CONFIG, OIDC_KEY_ERROR, IPC_KEY_INFO);
  if (CALL_GETJSONVALUES(res) < 0) {
    printError("Could not decode json: %s\n", res);
    printError("This seems to be a bug. Please hand in a bug report.\n");
    secFree(res);
    exit(EXIT_FAILURE);
  }
  KEY_VALUE_VARS(status, config, error, info);
  secFree(res);

  if (_error != NULL) {
    printError("Error: %s\n", pairs[2].value);
    if (_info) {
      printNormal("%s\n", pairs[3].value);
    }
    secFreeKeyValuePairs(pairs, sizeof(pairs) / sizeof(*pairs));
    exit(EXIT_FAILURE);
  }
  printNormal("%s\n", _status);
  secFree(_status);
  secFreeKeyValuePairs(&pairs[2], sizeof(pairs) / sizeof(*pairs) - 2);
  return _config;
}
