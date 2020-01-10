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
#include "utils/logger.h"
#include "utils/memory.h"
#include "utils/oidc_error.h"

int oidcd_main(struct ipcPipe pipes, const struct arguments* arguments) {
  logger_open("oidc-agent.d");
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
      logger(ERROR, "%s", oidc_serror());
      if (oidc_errno == OIDC_EIPCDIS) {
        exit(EXIT_FAILURE);
      }
      if (ipc_writeOidcErrnoToPipe(pipes) ==
          OIDC_SUCCESS) {  // Try to communicate the error back
        continue;
      }
      exit(EXIT_FAILURE);
    }
    INIT_KEY_VALUE(IPC_KEY_REQUEST, IPC_KEY_SHORTNAME, IPC_KEY_MINVALID,
                   IPC_KEY_CONFIG, IPC_KEY_FLOW, IPC_KEY_USECUSTOMSCHEMEURL,
                   IPC_KEY_REDIRECTEDURI, OIDC_KEY_STATE, IPC_KEY_AUTHORIZATION,
                   OIDC_KEY_SCOPE, IPC_KEY_DEVICE, IPC_KEY_FROMGEN,
                   IPC_KEY_LIFETIME, IPC_KEY_PASSWORD, IPC_KEY_APPLICATIONHINT,
                   IPC_KEY_CONFIRM, IPC_KEY_ISSUERURL, IPC_KEY_NOSCHEME,
                   IPC_KEY_CERTPATH);
    if (getJSONValuesFromString(q, pairs, sizeof(pairs) / sizeof(*pairs)) < 0) {
      ipc_writeToPipe(pipes, RESPONSE_BADREQUEST, oidc_serror());
      secFreeKeyValuePairs(pairs, sizeof(pairs) / sizeof(*pairs));
      secFree(q);
      continue;
    }
    secFree(q);
    KEY_VALUE_VARS(request, shortname, minvalid, config, flow, nowebserver,
                   redirectedUri, state, authorization, scope, device, fromGen,
                   lifetime, password, applicationHint, confirm, issuer,
                   noscheme,
                   cert_path);  // Gives variables for key_value values;
                                // e.g. _request=pairs[0].value
    if (_request == NULL) {
      ipc_writeToPipe(pipes, RESPONSE_BADREQUEST, "No request type.");
      secFreeKeyValuePairs(pairs, sizeof(pairs) / sizeof(*pairs));
      continue;
    }

    if (strequal(_request, REQUEST_VALUE_CHECK)) {  // Allow check in all cases
      ipc_writeToPipe(pipes, RESPONSE_SUCCESS);
      secFreeKeyValuePairs(pairs, sizeof(pairs) / sizeof(*pairs));
      continue;
    }
    if (agent_state.lock_state.locked) {  // If locked allow only unlock
      if (strequal(_request, REQUEST_VALUE_UNLOCK)) {
        oidcd_handleLock(pipes, _password, 0);
      } else {
        oidc_errno = OIDC_ELOCKED;
        ipc_writeOidcErrnoToPipe(pipes);
      }
      secFreeKeyValuePairs(pairs, sizeof(pairs) / sizeof(*pairs));
      continue;
    }
    if (strequal(_request, REQUEST_VALUE_GEN)) {
      oidcd_handleGen(pipes, _config, _flow, _nowebserver, _noscheme,
                      arguments);
    } else if (strequal(_request, REQUEST_VALUE_CODEEXCHANGE)) {
      oidcd_handleCodeExchange(pipes, _redirectedUri, _fromGen);
    } else if (strequal(_request, REQUEST_VALUE_STATELOOKUP)) {
      oidcd_handleStateLookUp(pipes, _state);
    } else if (strequal(_request, REQUEST_VALUE_DEVICELOOKUP)) {
      oidcd_handleDeviceLookup(pipes, _config, _device);
    } else if (strequal(_request, REQUEST_VALUE_ADD)) {
      oidcd_handleAdd(pipes, _config, _lifetime, _confirm);
    } else if (strequal(_request, REQUEST_VALUE_REMOVE)) {
      oidcd_handleRm(pipes, _shortname);
    } else if (strequal(_request, REQUEST_VALUE_REMOVEALL)) {
      oidcd_handleRemoveAll(pipes);
    } else if (strequal(_request, REQUEST_VALUE_DELETE)) {
      oidcd_handleDelete(pipes, _config);
    } else if (strequal(_request, REQUEST_VALUE_ACCESSTOKEN)) {
      if (_shortname) {
        oidcd_handleToken(pipes, _shortname, _minvalid, _scope,
                          _applicationHint, arguments);
      } else if (_issuer) {
        oidcd_handleTokenIssuer(pipes, _issuer, _minvalid, _scope,
                                _applicationHint, arguments);
      } else {
        // global default
        oidc_errno = OIDC_NOTIMPL;  // TODO
        ipc_writeOidcErrnoToPipe(pipes);
      }
    } else if (strequal(_request, REQUEST_VALUE_REGISTER)) {
      oidcd_handleRegister(pipes, _config, _flow, _authorization);
    } else if (strequal(_request, REQUEST_VALUE_TERMHTTP)) {
      oidcd_handleTermHttp(pipes, _state);
    } else if (strequal(_request, REQUEST_VALUE_SCOPES)) {
      oidcd_handleScopes(pipes, _issuer, _cert_path);
    } else if (strequal(_request, REQUEST_VALUE_LOADEDACCOUNTS)) {
      oidcd_handleListLoadedAccounts(pipes);
    } else if (strequal(_request, REQUEST_VALUE_LOCK)) {
      oidcd_handleLock(pipes, _password, 1);
    } else if (strequal(_request, REQUEST_VALUE_UNLOCK)) {
      oidc_errno = OIDC_ENOTLOCKED;
      ipc_writeOidcErrnoToPipe(pipes);
    } else {  // Unknown request type
      ipc_writeToPipe(pipes, RESPONSE_BADREQUEST, "Unknown request type.");
    }
    secFreeKeyValuePairs(pairs, sizeof(pairs) / sizeof(*pairs));
  }
  return EXIT_FAILURE;
}
