#include "parse_ipc.h"
#include "ipc/ipc_values.h"
#include "utils/json.h"
#include "utils/key_value.h"
#include "utils/pass.h"

/**
 * @param res a pointer to the response that should be parsed. The pointer will
 * be freed!
 * @param arguments the arguments oidc-token-exchange was called with
 */
char* exchange_parseResponse(char* res, struct arguments arguments) {
  struct key_value pairs[4];
  pairs[0].key = "status";
  pairs[1].key = "config";
  pairs[2].key = "error";
  pairs[3].key = "info";
  if (getJSONValuesFromString(res, pairs, sizeof(pairs) / sizeof(*pairs)) < 0) {
    printError("Could not decode json: %s\n", res);
    printError("This seems to be a bug. Please hand in a bug report.\n");
    secFree(res);
    exit(EXIT_FAILURE);
  }
  secFree(res);
  if (pairs[2].value != NULL) {
    printError("Error: %s\n", pairs[2].value);
    if (pairs[3].value) {
      printNormal("%s\n", pairs[3].value);
    }
    secFreeKeyValuePairs(pairs, sizeof(pairs) / sizeof(*pairs));
    exit(EXIT_FAILURE);
  }
  char* config = NULL;
  if (pairs[1].value != NULL) {  // res contains config
    config = pairs[1].value;
  } else {  // res does not contain config
    pass;
  }
  printNormal("%s\n", pairs[0].value);
  if (strequal(pairs[0].value, STATUS_SUCCESS)) {
    pass;
  }
  secFree(pairs[0].value);
  secFreeKeyValuePairs(&pairs[2], sizeof(pairs) / sizeof(*pairs) - 2);
  return config;
}
