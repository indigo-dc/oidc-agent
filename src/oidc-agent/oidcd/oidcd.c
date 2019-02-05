#include "oidcd.h"
#include "account/account.h"
#include "defines/ipc_values.h"
#include "list/list.h"
#include "oidc-agent/agent_state.h"
#include "oidc-agent/oidcd/oidcd_handler.h"
#include "utils/accountUtils.h"
#include "utils/crypt/crypt.h"
#include "utils/crypt/memoryCrypt.h"
#include "utils/json.h"
#include "utils/listUtils.h"
#include "utils/memory.h"
#include "utils/oidc_error.h"

#include <syslog.h>

int oidcd_main(struct ipcPipe pipes, const struct arguments* arguments) {
  openlog("oidc-agent.d", LOG_CONS | LOG_PID, LOG_AUTHPRIV);
  initCrypt();
  initMemoryCrypt();

  list_t* loaded_accounts = list_new();
  loaded_accounts->free   = (void (*)(void*)) & _secFreeAccount;
  loaded_accounts->match  = (int (*)(void*, void*)) & account_matchByName;
  time_t minDeath         = 0;

  while (1) {
    minDeath = getMinAccountDeath(loaded_accounts);
    char* q  = ipc_readFromPipeWithTimeout(pipes, minDeath);
    if (q == NULL) {
      if (oidc_errno == OIDC_ETIMEOUT) {
        struct oidc_account* death = NULL;
        while ((death = getDeathAccount(loaded_accounts)) != NULL) {
          list_remove(loaded_accounts, findInList(loaded_accounts, death));
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
    }  // q!=NULL
    size_t           size = 15;
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
    if (getJSONValuesFromString(q, pairs, sizeof(pairs) / sizeof(*pairs)) < 0) {
      ipc_writeToPipe(pipes, RESPONSE_BADREQUEST, oidc_serror());
    } else {
      if (pairs[0].value) {
        if (strcmp(pairs[0].value, REQUEST_VALUE_CHECK) == 0) {
          ipc_writeToPipe(pipes, RESPONSE_SUCCESS);
        } else if (agent_state.lock_state.locked) {
          if (strcmp(pairs[0].value, REQUEST_VALUE_UNLOCK) ==
              0) {  // the agent might be unlocked
            oidcd_handleLock(pipes, pairs[13].value, loaded_accounts, 0);
          } else {  // all other requests are not acceptable while locked
            oidc_errno = OIDC_ELOCKED;
            ipc_writeOidcErrnoToPipe(pipes);
          }
        } else {  // Agent not locked
          if (strcmp(pairs[0].value, REQUEST_VALUE_GEN) == 0) {
            oidcd_handleGen(pipes, loaded_accounts, pairs[3].value,
                            pairs[4].value);
          } else if (strcmp(pairs[0].value, REQUEST_VALUE_CODEEXCHANGE) == 0) {
            oidcd_handleCodeExchange(pipes, loaded_accounts, pairs[3].value,
                                     pairs[5].value, pairs[6].value,
                                     pairs[7].value, pairs[11].value);
          } else if (strcmp(pairs[0].value, REQUEST_VALUE_STATELOOKUP) == 0) {
            oidcd_handleStateLookUp(pipes, loaded_accounts, pairs[7].value);
          } else if (strcmp(pairs[0].value, REQUEST_VALUE_DEVICELOOKUP) == 0) {
            oidcd_handleDeviceLookup(pipes, loaded_accounts, pairs[3].value,
                                     pairs[10].value);
          } else if (strcmp(pairs[0].value, REQUEST_VALUE_ADD) == 0) {
            oidcd_handleAdd(pipes, loaded_accounts, pairs[3].value,
                            pairs[12].value);
          } else if (strcmp(pairs[0].value, REQUEST_VALUE_REMOVE) == 0) {
            oidcd_handleRm(pipes, loaded_accounts, pairs[1].value);
          } else if (strcmp(pairs[0].value, REQUEST_VALUE_REMOVEALL) == 0) {
            oidcd_handleRemoveAll(pipes, &loaded_accounts);
          } else if (strcmp(pairs[0].value, REQUEST_VALUE_DELETE) == 0) {
            oidcd_handleDelete(pipes, loaded_accounts, pairs[3].value);
          } else if (strcmp(pairs[0].value, REQUEST_VALUE_ACCESSTOKEN) == 0) {
            oidcd_handleToken(pipes, loaded_accounts, pairs[1].value,
                              pairs[2].value, pairs[9].value, pairs[14].value);
          } else if (strcmp(pairs[0].value, REQUEST_VALUE_REGISTER) == 0) {
            oidcd_handleRegister(pipes, loaded_accounts, pairs[3].value,
                                 pairs[4].value, pairs[8].value);
          } else if (strcmp(pairs[0].value, REQUEST_VALUE_TERMHTTP) == 0) {
            oidcd_handleTermHttp(pipes, pairs[7].value);
          } else if (strcmp(pairs[0].value, REQUEST_VALUE_LOCK) == 0) {
            oidcd_handleLock(pipes, pairs[13].value, loaded_accounts, 1);
          } else if (strcmp(pairs[0].value, REQUEST_VALUE_UNLOCK) == 0) {
            oidc_errno = OIDC_ENOTLOCKED;
            ipc_writeOidcErrnoToPipe(pipes);
          } else {
            ipc_writeToPipe(pipes, RESPONSE_BADREQUEST,
                            "Unknown request type.");
          }
        }
      } else {
        ipc_writeToPipe(pipes, RESPONSE_BADREQUEST, "No request type.");
      }
      secFreeKeyValuePairs(pairs, sizeof(pairs) / sizeof(*pairs));
      secFree(q);
    }
  }
  return EXIT_FAILURE;
}
