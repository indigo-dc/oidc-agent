#include "parse_ipc.h"
#include "defines/ipc_values.h"
#include "utils/json.h"
#include "utils/key_value.h"
#include "utils/memory.h"
#include "utils/printer.h"
#include "utils/stringUtils.h"

#include <stdlib.h>

void add_parseResponse(char* res) {
  if (NULL == res) {
    printError("Error: %s\n", oidc_serror());
    exit(EXIT_FAILURE);
  }
  INIT_KEY_VALUE(IPC_KEY_STATUS, IPC_KEY_INFO, OIDC_KEY_ERROR);
  if (CALL_GETJSONVALUES(res) < 0) {
    printError("Could not decode json: %s\n", res);
    printError("This seems to be a bug. Please hand in a bug report.\n");
    secFree(res);
    SEC_FREE_KEY_VALUES();
    exit(EXIT_FAILURE);
  }
  secFree(res);
  KEY_VALUE_VARS(status, info, error);
  if (_error != NULL) {
    printError("Error: %s\n", _error);
    SEC_FREE_KEY_VALUES();
    exit(EXIT_FAILURE);
  }
  printf("%s\n", _status);
  if (strValid(_info)) {
    printf("%s\n", _info);
  }
  SEC_FREE_KEY_VALUES();
}
