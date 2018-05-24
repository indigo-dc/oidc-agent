#include "json.h"
#include "device_code.h"
#include "utils/stringUtils.h"

#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>

char* parseForError(char* res) {
  struct key_value pairs[2];
  pairs[0].key = "error";
  pairs[1].key = "error_description";
  if(getJSONValues(res, pairs, sizeof(pairs)/sizeof(*pairs))<0) {
    printError("Could not decode json: %s\n", res);
    printError("This seems to be a bug. Please hand in a bug report.\n");
    clearFreeString(res);
    return NULL;
  }
  clearFreeString(res);

  if(pairs[1].value) {
    char* error = oidc_sprintf("%s: %s", pairs[0].value, pairs[1].value);
    clearFreeKeyValuePairs(pairs, sizeof(pairs)/sizeof(*pairs));
    return error;
  }
  return pairs[0].value;
}

struct oidc_device_code* parseDeviceCode(char* res) {
  if(!isJSONObject(res)) {
    return NULL;
  }
  char* copy = oidc_strcopy(res);
  char* error = parseForError(copy);
  if(error) {
    oidc_seterror(error);
    oidc_errno = OIDC_EOIDC;
    clearFreeString(error);
    return NULL;
  }
  return getDeviceCodeFromJSON(res);  
}
