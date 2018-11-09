#include "parse_ipc.h"
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

  struct key_value pairs[3];
  pairs[0].key = "status";
  pairs[1].key = "info";
  pairs[2].key = "error";
  if (getJSONValuesFromString(res, pairs, sizeof(pairs) / sizeof(*pairs)) < 0) {
    printError("Could not decode json: %s\n", res);
    printError("This seems to be a bug. Please hand in a bug report.\n");
    secFree(res);
    exit(EXIT_FAILURE);
  }
  secFree(res);
  if (pairs[2].value != NULL) {
    printError("Error: %s\n", pairs[2].value);
    secFreeKeyValuePairs(pairs, sizeof(pairs) / sizeof(*pairs));
    exit(EXIT_FAILURE);
  }
  printf("%s\n", pairs[0].value);
  if (strValid(pairs[1].value)) {
    printf("%s\n", pairs[1].value);
  }
  secFreeKeyValuePairs(pairs, sizeof(pairs) / sizeof(*pairs));
}
