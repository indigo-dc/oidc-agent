#include "token_handler.h"

#include "defines/ipc_values.h"
#include "ipc/communicator.h"
#include "utils/json.h"
#include "utils/key_value.h"
#include "utils/oidc_error.h"
#include "utils/printer.h"

#include <stdlib.h>

void token_handleIdToken(const unsigned char useIssuerInsteadOfShortname,
                         const char*         name) {
  char* response =
      ipc_communicate(useIssuerInsteadOfShortname ? REQUEST_IDTOKEN_ISSUER
                                                  : REQUEST_IDTOKEN_ACCOUNT,
                      name, "oidc-token");
  if (response == NULL) {
    oidc_perror();
    exit(EXIT_FAILURE);
  }
  INIT_KEY_VALUE(/* IPC_KEY_STATUS, */ OIDC_KEY_ERROR, OIDC_KEY_IDTOKEN);
  if (CALL_GETJSONVALUES(response) < 0) {
    printError("%s:%lu Read malformed data. Please hand in bug report.\n",
               __FILE__, __LINE__);
    secFree(response);
    SEC_FREE_KEY_VALUES();
    exit(EXIT_FAILURE);
  }
  secFree(response);
  KEY_VALUE_VARS(/* status,  */ error, id_token);
  if (_error) {  // error
    printError("%s\n", _error);
    SEC_FREE_KEY_VALUES();
    exit(EXIT_FAILURE);
  }
  printStdout("%s\n", _id_token);
  SEC_FREE_KEY_VALUES();
}
