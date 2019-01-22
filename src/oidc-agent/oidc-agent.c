#define _XOPEN_SOURCE 500

#include "oidc-agent.h"
#include "account/account.h"
#include "agent_handler.h"
#include "agent_state.h"
#include "ipc/connection.h"
#include "ipc/cryptIpc.h"
#include "ipc/ipc.h"
#include "ipc/ipc_async.h"
#include "ipc/ipc_values.h"
#include "privileges/agent_privileges.h"
#include "settings.h"
#include "utils/accountUtils.h"
#include "utils/crypt/memoryCrypt.h"
#include "utils/listUtils.h"
#include "utils/oidc_error.h"
#include "utils/printer.h"

#include <fcntl.h>
#include <libgen.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <syslog.h>
#include <time.h>
#include <unistd.h>

int main(int argc, char** argv) {
  while (1) {
    minDeath               = getMinDeath(loaded_accounts);
    struct connection* con = ipc_async(*listencon, clientcons, minDeath);
    if (con == NULL) {  // timeout reached
      struct oidc_account* death = NULL;
      while ((death = getDeathAccount(loaded_accounts)) != NULL) {
        list_remove(loaded_accounts, findInList(loaded_accounts, death));
      }
      continue;
    } else {
      char* q = server_ipc_readFromSocket(*(con->msgsock));
      if (NULL != q) {
        size_t           size = 15;
        struct key_value pairs[size];
        for (size_t i = 0; i < size; i++) { pairs[i].value = NULL; }
        pairs[0].key  = "request";
        pairs[1].key  = "account";
        pairs[2].key  = "min_valid_period";
        pairs[3].key  = "config";
        pairs[4].key  = "flow";
        pairs[5].key  = "code";
        pairs[6].key  = "redirect_uri";
        pairs[7].key  = "state";
        pairs[8].key  = "authorization";
        pairs[9].key  = "scope";
        pairs[10].key = "oidc_device";
        pairs[11].key = "code_verifier";
        pairs[12].key = "lifetime";
        pairs[13].key = "password";
        pairs[14].key = "application_hint";
        if (getJSONValuesFromString(q, pairs, sizeof(pairs) / sizeof(*pairs)) <
            0) {
          server_ipc_write(*(con->msgsock), RESPONSE_BADREQUEST, oidc_serror());
        } else {
          if (pairs[0].value) {
            if (strcmp(pairs[0].value, REQUEST_VALUE_CHECK) == 0) {
              server_ipc_write(*(con->msgsock), RESPONSE_SUCCESS);
            } else if (agent_state.lock_state.locked) {
              if (strcmp(pairs[0].value, REQUEST_VALUE_UNLOCK) ==
                  0) {  // the agent might be unlocked
                agent_handleLock(*(con->msgsock), pairs[13].value,
                                 loaded_accounts, 0);
              } else {  // all other requests are not acceptable while locked
                oidc_errno = OIDC_ELOCKED;
                ipc_writeOidcErrno(*(con->msgsock));
              }
            } else {  // Agent not locked
              if (strcmp(pairs[0].value, REQUEST_VALUE_GEN) == 0) {
                agent_handleGen(*(con->msgsock), loaded_accounts,
                                pairs[3].value, pairs[4].value);
              } else if (strcmp(pairs[0].value, REQUEST_VALUE_CODEEXCHANGE) ==
                         0) {
                agent_handleCodeExchange(*(con->msgsock), loaded_accounts,
                                         pairs[3].value, pairs[5].value,
                                         pairs[6].value, pairs[7].value,
                                         pairs[11].value);
              } else if (strcmp(pairs[0].value, REQUEST_VALUE_STATELOOKUP) ==
                         0) {
                agent_handleStateLookUp(*(con->msgsock), loaded_accounts,
                                        pairs[7].value);
              } else if (strcmp(pairs[0].value, REQUEST_VALUE_DEVICELOOKUP) ==
                         0) {
                agent_handleDeviceLookup(*(con->msgsock), loaded_accounts,
                                         pairs[3].value, pairs[10].value);
              } else if (strcmp(pairs[0].value, REQUEST_VALUE_ADD) == 0) {
                agent_handleAdd(*(con->msgsock), loaded_accounts,
                                pairs[3].value, pairs[12].value);
              } else if (strcmp(pairs[0].value, REQUEST_VALUE_REMOVE) == 0) {
                agent_handleRm(*(con->msgsock), loaded_accounts,
                               pairs[1].value);
              } else if (strcmp(pairs[0].value, REQUEST_VALUE_REMOVEALL) == 0) {
                agent_handleRemoveAll(*(con->msgsock), &loaded_accounts);
              } else if (strcmp(pairs[0].value, REQUEST_VALUE_DELETE) == 0) {
                agent_handleDelete(*(con->msgsock), loaded_accounts,
                                   pairs[3].value);
              } else if (strcmp(pairs[0].value, REQUEST_VALUE_ACCESSTOKEN) ==
                         0) {
                agent_handleToken(*(con->msgsock), loaded_accounts,
                                  pairs[1].value, pairs[2].value,
                                  pairs[9].value, pairs[14].value);
              } else if (strcmp(pairs[0].value, REQUEST_VALUE_REGISTER) == 0) {
                agent_handleRegister(*(con->msgsock), loaded_accounts,
                                     pairs[3].value, pairs[4].value,
                                     pairs[8].value);
              } else if (strcmp(pairs[0].value, REQUEST_VALUE_TERMHTTP) == 0) {
                agent_handleTermHttp(*(con->msgsock), pairs[7].value);
              } else if (strcmp(pairs[0].value, REQUEST_VALUE_LOCK) == 0) {
                agent_handleLock(*(con->msgsock), pairs[13].value,
                                 loaded_accounts, 1);
              } else if (strcmp(pairs[0].value, REQUEST_VALUE_UNLOCK) == 0) {
                oidc_errno = OIDC_ENOTLOCKED;
                server_ipc_writeOidcErrno(*(con->msgsock));
              } else {
                server_ipc_write(*(con->msgsock), RESPONSE_BADREQUEST,
                                 "Unknown request type.");
              }
            }
          } else {
            server_ipc_write(*(con->msgsock), RESPONSE_BADREQUEST,
                             "No request type.");
          }
        }
        secFreeKeyValuePairs(pairs, sizeof(pairs) / sizeof(*pairs));
        secFree(q);
      }
      syslog(LOG_AUTHPRIV | LOG_DEBUG, "Remove con from pool");
      list_remove(clientcons, findInList(clientcons, con));
      syslog(LOG_AUTHPRIV | LOG_DEBUG, "Currently there are %d connections",
             clientcons->len);
    }
  }
  return EXIT_FAILURE;
}
