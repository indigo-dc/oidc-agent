#include "oidcd.h"
#include "account/account.h"
#include "defines/ipc_values.h"
#include "list/list.h"
#include "oidc-agent/agent_state.h"
#include "oidc-agent/oidcd/codeExchangeEntry.h"
#include "oidc-agent/oidcd/oidcd_handler.h"
#include "utils/accountUtils.h"
#include "utils/crypt/crypt.h"
#include "utils/crypt/memoryCrypt.h"
#include "utils/db/account_db.h"
#include "utils/db/codeVerifier_db.h"
#include "utils/json.h"
#include "utils/listUtils.h"
#include "utils/memory.h"
#include "utils/oidc_error.h"

#include <syslog.h>

int oidcd_main(struct ipcPipe pipes, const struct arguments* arguments) {
  openlog("oidc-agent.d", LOG_CONS | LOG_PID, LOG_AUTHPRIV);
  initCrypt();
  initMemoryCrypt();

  codeVerifierDB_new();
  codeVerifierDB_setFreeFunction((freeFunction)_secFree);
  codeVerifierDB_setMatchFunction((matchFunction)cee_matchByState);

  accountDB_new();
  accountDB_setFreeFunction((freeFunction)_secFreeAccount);
  accountDB_setMatchFunction((matchFunction)account_matchByName);
  time_t minDeath = 0;

  while (1) {
    minDeath = getMinAccountDeath();
    char* q  = ipc_readFromPipeWithTimeout(pipes, minDeath);
    if (q == NULL) {
      if (oidc_errno == OIDC_ETIMEOUT) {
        struct oidc_account* death = NULL;
        while ((death = getDeathAccount()) != NULL) {
          accountDB_removeIfFound(death);
        }
        continue;
      }  // A real error and no timeout
      syslog(LOG_AUTHPRIV | LOG_ERR, "%s", oidc_serror());
      if (oidc_errno == OIDC_EIPCDIS) {
        exit(EXIT_FAILURE);
      }
      if (ipc_writeOidcErrnoToPipe(pipes) ==
          OIDC_SUCCESS) {  // Try to communicate the error back
        continue;
      }
      exit(EXIT_FAILURE);
    }
    size_t           size = 16;
    struct key_value pairs[size];
    for (size_t i = 0; i < size; i++) { pairs[i].value = NULL; }
    pairs[0].key  = IPC_KEY_REQUEST;
    pairs[1].key  = IPC_KEY_SHORTNAME;
    pairs[2].key  = IPC_KEY_MINVALID;
    pairs[3].key  = IPC_KEY_CONFIG;
    pairs[4].key  = IPC_KEY_FLOW;
    pairs[5].key  = OIDC_KEY_CODE;
    pairs[6].key  = OIDC_KEY_REDIRECTURI;
    pairs[7].key  = OIDC_KEY_STATE;
    pairs[8].key  = IPC_KEY_AUTHORIZATION;
    pairs[9].key  = OIDC_KEY_SCOPE;
    pairs[10].key = IPC_KEY_DEVICE;
    pairs[11].key = OIDC_KEY_CODEVERIFIER;
    pairs[12].key = IPC_KEY_LIFETIME;
    pairs[13].key = IPC_KEY_PASSWORD;
    pairs[14].key = IPC_KEY_APPLICATIONHINT;
    pairs[15].key = IPC_KEY_CONFIRM;
    if (getJSONValuesFromString(q, pairs, sizeof(pairs) / sizeof(*pairs)) < 0) {
      ipc_writeToPipe(pipes, RESPONSE_BADREQUEST, oidc_serror());
      secFreeKeyValuePairs(pairs, sizeof(pairs) / sizeof(*pairs));
      secFree(q);
      continue;
    }
    secFree(q);
    char* request = pairs[0].value;
    if (request == NULL) {
      ipc_writeToPipe(pipes, RESPONSE_BADREQUEST, "No request type.");
      secFreeKeyValuePairs(pairs, sizeof(pairs) / sizeof(*pairs));
      continue;
    }

    if (strequal(request, REQUEST_VALUE_CHECK)) {  // Allow check in all cases
      ipc_writeToPipe(pipes, RESPONSE_SUCCESS);
      secFreeKeyValuePairs(pairs, sizeof(pairs) / sizeof(*pairs));
      continue;
    }
    if (agent_state.lock_state.locked) {  // If locked allow only unlock
      if (strequal(request, REQUEST_VALUE_UNLOCK)) {
        oidcd_handleLock(pipes, pairs[13].value, 0);
      } else {
        oidc_errno = OIDC_ELOCKED;
        ipc_writeOidcErrnoToPipe(pipes);
      }
      secFreeKeyValuePairs(pairs, sizeof(pairs) / sizeof(*pairs));
      continue;
    }
    if (strequal(request, REQUEST_VALUE_GEN)) {
      oidcd_handleGen(pipes, pairs[3].value, pairs[4].value);
    } else if (strequal(request, REQUEST_VALUE_CODEEXCHANGE)) {
      oidcd_handleCodeExchange(pipes, pairs[3].value, pairs[5].value,
                               pairs[6].value, pairs[7].value, pairs[11].value);
    } else if (strequal(request, REQUEST_VALUE_STATELOOKUP)) {
      oidcd_handleStateLookUp(pipes, pairs[7].value);
    } else if (strequal(request, REQUEST_VALUE_DEVICELOOKUP)) {
      oidcd_handleDeviceLookup(pipes, pairs[3].value, pairs[10].value);
    } else if (strequal(request, REQUEST_VALUE_ADD)) {
      oidcd_handleAdd(pipes, pairs[3].value, pairs[12].value, pairs[15].value);
    } else if (strequal(request, REQUEST_VALUE_REMOVE)) {
      oidcd_handleRm(pipes, pairs[1].value);
    } else if (strequal(request, REQUEST_VALUE_REMOVEALL)) {
      oidcd_handleRemoveAll(pipes);
    } else if (strequal(request, REQUEST_VALUE_DELETE)) {
      oidcd_handleDelete(pipes, pairs[3].value);
    } else if (strequal(request, REQUEST_VALUE_ACCESSTOKEN)) {
      oidcd_handleToken(pipes, pairs[1].value, pairs[2].value, pairs[9].value,
                        pairs[14].value, arguments);
    } else if (strequal(request, REQUEST_VALUE_REGISTER)) {
      oidcd_handleRegister(pipes, pairs[3].value, pairs[4].value,
                           pairs[8].value);
    } else if (strequal(request, REQUEST_VALUE_TERMHTTP)) {
      oidcd_handleTermHttp(pipes, pairs[7].value);
    } else if (strequal(request, REQUEST_VALUE_LOCK)) {
      oidcd_handleLock(pipes, pairs[13].value, 1);
    } else if (strequal(request, REQUEST_VALUE_UNLOCK)) {
      oidc_errno = OIDC_ENOTLOCKED;
      ipc_writeOidcErrnoToPipe(pipes);
    } else {  // Unknown request type
      ipc_writeToPipe(pipes, RESPONSE_BADREQUEST, "Unknown request type.");
    }
    secFreeKeyValuePairs(pairs, sizeof(pairs) / sizeof(*pairs));
  }
  return EXIT_FAILURE;
}
