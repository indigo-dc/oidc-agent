#include "parseJson.h"

#include "defines/oidc_values.h"
#include "utils/errorUtils.h"
#include "utils/json.h"
#include "utils/printer.h"

char* parseForError(char* res) {
  INIT_KEY_VALUE(OIDC_KEY_ERROR, OIDC_KEY_ERROR_DESCRIPTION);
  if (CALL_GETJSONVALUES(res) < 0) {
    printError("Could not decode json: %s\n", res);
    printError("This seems to be a bug. Please hand in a bug report.\n");
    secFree(res);
    SEC_FREE_KEY_VALUES();
    return NULL;
  }
  secFree(res);
  KEY_VALUE_VARS(error, error_description);
  if (_error_description) {
    char* error = combineError(_error, _error_description);
    SEC_FREE_KEY_VALUES();
    return error;
  }
  return _error;
}
