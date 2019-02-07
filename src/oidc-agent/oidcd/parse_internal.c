#include "parse_internal.h"
#include "defines/ipc_values.h"
#include "utils/json.h"
#include "utils/memory.h"
#include "utils/oidc_error.h"

char* parseForConfig(char* res) {
  struct key_value pairs[2];
  pairs[0].key = INT_IPC_KEY_OIDCERRNO;
  pairs[1].key = IPC_KEY_CONFIG;
  if (getJSONValuesFromString(res, pairs, sizeof(pairs) / sizeof(*pairs)) < 0) {
    printError("Could not decode json: %s\n", res);
    printError("This seems to be a bug. Please hand in a bug report.\n");
    secFree(res);
    return NULL;
  }
  secFree(res);

  if (pairs[0].value) {
    oidc_errno = strToInt(pairs[0].value);
    secFree(pairs[0].value);
  }
  return pairs[1].value;
}
