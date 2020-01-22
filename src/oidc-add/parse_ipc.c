#include "parse_ipc.h"
#include "defines/ipc_values.h"
#include "utils/json.h"
#include "utils/key_value.h"
#include "utils/listUtils.h"
#include "utils/memory.h"
#include "utils/printer.h"
#include "utils/stringUtils.h"

#include <stdlib.h>
struct statusInfo {
  char* status;
  char* info;
};

void secFreeStatusInfo(struct statusInfo a) {
  secFree(a.status);
  secFree(a.info);
}

struct statusInfo _add_parseResponse(char* res) {
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
  secFree(_error);
  return (struct statusInfo){_status, _info};
}

void add_parseResponse(char* res) {
  struct statusInfo tmp = _add_parseResponse(res);
  printStdout("%s\n", tmp.status);
  if (strValid(tmp.info)) {
    printNormal("%s\n", tmp.info);
  }
  secFreeStatusInfo(tmp);
}

void add_parseLoadedAccountsResponse(char* res) {
  struct statusInfo tmp = _add_parseResponse(res);
  char* printable       = JSONArrayStringToDelimitedString(tmp.info, '\n');
  secFreeStatusInfo(tmp);
  printStdout("%s\n", printable);
  secFree(printable);
}
