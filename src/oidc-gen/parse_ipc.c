#define _XOPEN_SOURCE 500
#include "parse_ipc.h"
#include "defines/ipc_values.h"
#include "defines/oidc_values.h"
#include "oidc-gen/gen_handler.h"
#include "oidc-gen/gen_signal_handler.h"
#include "utils/json.h"
#include "utils/key_value.h"
#include "utils/memory.h"
#include "utils/printer.h"
#include "utils/stringUtils.h"

#include <string.h>
#include <strings.h>
#include <syslog.h>
#include <unistd.h>

/**
 * @param res a pointer to the response that should be parsed. The pointer will
 * be freed!
 */
char* gen_parseResponse(char* res, const struct arguments* arguments) {
  struct key_value pairs[7];
  pairs[0].key = IPC_KEY_STATUS;
  pairs[1].key = IPC_KEY_CONFIG;
  pairs[2].key = OIDC_KEY_ERROR;
  pairs[3].key = IPC_KEY_URI;
  pairs[4].key = IPC_KEY_INFO;
  pairs[5].key = OIDC_KEY_STATE;
  pairs[6].key = IPC_KEY_DEVICE;
  if (getJSONValuesFromString(res, pairs, sizeof(pairs) / sizeof(*pairs)) < 0) {
    printError("Could not decode json: %s\n", res);
    printError("This seems to be a bug. Please hand in a bug report.\n");
    secFree(res);
    exit(EXIT_FAILURE);
  }
  secFree(res);
  if (pairs[2].value != NULL) {
    printError("Error: %s\n", pairs[2].value);
    if (pairs[4].value) {
      printf("%s\n", pairs[4].value);
    }
    secFreeKeyValuePairs(pairs, sizeof(pairs) / sizeof(*pairs));
    exit(EXIT_FAILURE);
  }
  char* config = NULL;
  if (pairs[1].value != NULL) {  // res contains config
    config = pairs[1].value;
  } else {  // res does not contain config
    if (strcaseequal(pairs[0].value, STATUS_NOTFOUND)) {
      syslog(LOG_AUTHPRIV | LOG_DEBUG, "%s", pairs[4].value);
      secFreeKeyValuePairs(pairs, sizeof(pairs) / sizeof(*pairs));
      return NULL;
    }
    if (pairs[3].value == NULL) {
      printError("Error: response does not contain updated config\n");
    }
  }
  printf("%s\n", pairs[0].value);
  if (strcmp(pairs[0].value, STATUS_SUCCESS) == 0) {
    printf("The generated account config was successfully added to oidc-agent. "
           "You don't have to run oidc-add.\n");
  } else if (strcaseequal(pairs[0].value, STATUS_ACCEPTED)) {
    if (pairs[4].value) {
      printImportant("%s\n", pairs[4].value);
    }
    if (pairs[6].value) {
      char* ret = gen_handleDeviceFlow(pairs[6].value, config, arguments);
      secFreeKeyValuePairs(pairs, sizeof(pairs) / sizeof(*pairs));
      return ret;
    }
    if (pairs[3].value) {
      if (pairs[5].value) {
        registerSignalHandler(pairs[5].value);
      }
      printImportant("To continue and approve the registered client visit the "
                     "following URL in a Browser of your choice:\n%s\n",
                     pairs[3].value);
      char* cmd = oidc_sprintf("xdg-open \"%s\"", pairs[3].value);
      system(cmd);
      secFree(cmd);
    }
    if (pairs[5].value) {
      sleep(2);
      handleStateLookUp(pairs[5].value, arguments);
    }
  }
  secFree(pairs[0].value);
  secFreeKeyValuePairs(&pairs[2], sizeof(pairs) / sizeof(*pairs) - 2);
  return config;
}
