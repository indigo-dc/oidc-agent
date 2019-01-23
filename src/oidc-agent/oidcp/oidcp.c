#define _XOPEN_SOURCE 500

#include "oidcp.h"
#include "ipc/cryptIpc.h"
#include "ipc/ipc_values.h"
#include "ipc/pipe.h"
#include "ipc/serveripc.h"
#include "list/list.h"
#include "oidc-agent/daemonize.h"
#include "oidc-agent/oidcd/oidcd.h"
#include "privileges/agent_privileges.h"
#include "settings.h"
#include "utils/crypt/crypt.h"
#include "utils/disableTracing.h"
#include "utils/json.h"
#include "utils/listUtils.h"
#include "utils/memory.h"
#include "utils/printer.h"
#include "utils/stringUtils.h"

#include <libgen.h>
#include <signal.h>
#include <sys/prctl.h>
#include <syslog.h>
#include <time.h>
#include <unistd.h>

struct ipcPipe startOidcd(const struct arguments* arguments) {
  struct pipeSet pipes = ipc_pipe_init();
  if (pipes.pipe1.rx == -1) {
    syslog(LOG_AUTHPRIV | LOG_ERR, "could not create pipes");
    exit(EXIT_FAILURE);
  }
  pid_t ppid_before_fork = getpid();
  pid_t pid              = fork();
  if (pid == -1) {
    syslog(LOG_AUTHPRIV | LOG_ERR, "fork %m");
    exit(EXIT_FAILURE);
  }
  if (pid == 0) {  // child
    int r = prctl(PR_SET_PDEATHSIG, SIGTERM);
    if (r == -1) {
      syslog(LOG_AUTHPRIV | LOG_ERR, "prctl %m");
      exit(EXIT_FAILURE);
    }
    // test in case the original parent exited just before the prctl() call
    if (getppid() != ppid_before_fork) {
      syslog(LOG_AUTHPRIV | LOG_ERR, "Parent died shortly after fork");
      exit(EXIT_FAILURE);
    }
    struct ipcPipe childPipes = toClientPipes(pipes);
    oidcd_main(childPipes, arguments);
    exit(EXIT_FAILURE);
  } else {  // parent
    struct ipcPipe parentPipes = toServerPipes(pipes);
    return parentPipes;
  }
}

int main(int argc, char** argv) {
  platform_disable_tracing();
  openlog("oidc-agent.p", LOG_CONS | LOG_PID, LOG_AUTHPRIV);
  setlogmask(LOG_UPTO(LOG_NOTICE));
  struct arguments arguments;

  /* Set argument defaults */
  initArguments(&arguments);
  srandom(time(NULL));

  argp_parse(&argp, argc, argv, 0, 0, &arguments);
  if (arguments.debug) {
    setlogmask(LOG_UPTO(LOG_DEBUG));
  }
  if (arguments.seccomp) {
    initOidcAgentPrivileges(&arguments);
  }
  initCrypt();
  if (arguments.kill_flag) {
    char* pidstr = getenv(OIDC_PID_ENV_NAME);
    if (pidstr == NULL) {
      printError("%s not set, cannot kill Agent\n", OIDC_PID_ENV_NAME);
      exit(EXIT_FAILURE);
    }
    pid_t pid = strToInt(pidstr);
    if (0 == pid) {
      printError("%s not set to a valid pid: %s\n", OIDC_PID_ENV_NAME, pidstr);
      exit(EXIT_FAILURE);
    }
    if (kill(pid, SIGTERM) == -1) {
      perror("kill");
      exit(EXIT_FAILURE);
    } else {
      unlink(getenv(OIDC_SOCK_ENV_NAME));
      rmdir(dirname(getenv(OIDC_SOCK_ENV_NAME)));
      printf("unset %s;\n", OIDC_SOCK_ENV_NAME);
      printf("unset %s;\n", OIDC_PID_ENV_NAME);
      printf("echo Agent pid %d killed;\n", pid);
      exit(EXIT_SUCCESS);
    }
  }

  struct connection* listencon = secAlloc(sizeof(struct connection));
  if (ipc_server_init(listencon, OIDC_SOCK_ENV_NAME) != OIDC_SUCCESS) {
    printError("%s\n", oidc_serror());
    exit(EXIT_FAILURE);
  }
  if (!arguments.console) {
    daemonize();
  }

  struct ipcPipe pipes = startOidcd(&arguments);

  ipc_bindAndListen(listencon);

  list_t* clientcons = list_new();
  clientcons->free   = (void (*)(void*)) & _secFreeConnection;
  clientcons->match  = (int (*)(void*, void*)) & connection_comparator;

  while (1) {
    struct connection* con =
        ipc_readAsyncFromMultipleConnections(*listencon, clientcons);
    if (con == NULL) {  // timeout reached
      // TODO
      // does not happen
    } else {
      char* q = server_ipc_read(*(con->msgsock));
      if (NULL != q) {
        size_t           size = 15;
        struct key_value pairs[size];
        for (size_t i = 0; i < size; i++) { pairs[i].value = NULL; }
        pairs[0].key = "request";
        // pairs[1].key  = "account";
        // pairs[2].key  = "min_valid_period";
        // pairs[3].key  = "config";
        // pairs[4].key  = "flow";
        // pairs[5].key  = "code";
        // pairs[6].key  = "redirect_uri";
        // pairs[7].key  = "state";
        // pairs[8].key  = "authorization";
        // pairs[9].key  = "scope";
        // pairs[10].key = "oidc_device";
        // pairs[11].key = "code_verifier";
        // pairs[12].key = "lifetime";
        // pairs[13].key = "password";
        // pairs[14].key = "application_hint";
        if (getJSONValuesFromString(
                q, pairs, 1 /* sizeof(pairs) / sizeof(*pairs) */) < 0) {
          server_ipc_write(*(con->msgsock), RESPONSE_BADREQUEST, oidc_serror());
        } else {
          if (pairs[0].value) {
            // TODO forward complete request to oidcd
            char* oidcd_res = ipc_communicateThroughPipe(pipes, q);
            if (oidcd_res == NULL) {
              // TODO
              // if oidcd died
              // {
              //   syslog(LOG_AUTHPRIV | LOG_ERR, "oidcd died");
              //   exit(EXIT_FAILURE);
              // }
              // else {
              //   server_ipc_writeOidcErrno(*(con->msgsock));
              // }
            } else {
              server_ipc_write(*(con->msgsock), oidcd_res);
              secFree(oidcd_res);
            }

            // if (strcmp(pairs[0].value, REQUEST_VALUE_CHECK) == 0) {
            //   server_ipc_write(*(con->msgsock), RESPONSE_SUCCESS);
            // } else if (agent_state.lock_state.locked) {
            //   if (strcmp(pairs[0].value, REQUEST_VALUE_UNLOCK) ==
            //       0) {  // the agent might be unlocked
            //     agent_handleLock(*(con->msgsock), pairs[13].value,
            //                      loaded_accounts, 0);
            //   } else {  // all other requests are not acceptable while locked
            //     oidc_errno = OIDC_ELOCKED;
            //     ipc_writeOidcErrno(*(con->msgsock));
            //   }
            // } else {  // Agent not locked
            //   if (strcmp(pairs[0].value, REQUEST_VALUE_GEN) == 0) {
            //     agent_handleGen(*(con->msgsock), loaded_accounts,
            //                     pairs[3].value, pairs[4].value);
            //   } else if (strcmp(pairs[0].value, REQUEST_VALUE_CODEEXCHANGE)
            //   ==
            //              0) {
            //     agent_handleCodeExchange(*(con->msgsock), loaded_accounts,
            //                              pairs[3].value, pairs[5].value,
            //                              pairs[6].value, pairs[7].value,
            //                              pairs[11].value);
            //   } else if (strcmp(pairs[0].value, REQUEST_VALUE_STATELOOKUP) ==
            //              0) {
            //     agent_handleStateLookUp(*(con->msgsock), loaded_accounts,
            //                             pairs[7].value);
            //   } else if (strcmp(pairs[0].value, REQUEST_VALUE_DEVICELOOKUP)
            //   ==
            //              0) {
            //     agent_handleDeviceLookup(*(con->msgsock), loaded_accounts,
            //                              pairs[3].value, pairs[10].value);
            //   } else if (strcmp(pairs[0].value, REQUEST_VALUE_ADD) == 0) {
            //     agent_handleAdd(*(con->msgsock), loaded_accounts,
            //                     pairs[3].value, pairs[12].value);
            //   } else if (strcmp(pairs[0].value, REQUEST_VALUE_REMOVE) == 0) {
            //     agent_handleRm(*(con->msgsock), loaded_accounts,
            //                    pairs[1].value);
            //   } else if (strcmp(pairs[0].value, REQUEST_VALUE_REMOVEALL) ==
            //   0) {
            //     agent_handleRemoveAll(*(con->msgsock), &loaded_accounts);
            //   } else if (strcmp(pairs[0].value, REQUEST_VALUE_DELETE) == 0) {
            //     agent_handleDelete(*(con->msgsock), loaded_accounts,
            //                        pairs[3].value);
            //   } else if (strcmp(pairs[0].value, REQUEST_VALUE_ACCESSTOKEN) ==
            //              0) {
            //     agent_handleToken(*(con->msgsock), loaded_accounts,
            //                       pairs[1].value, pairs[2].value,
            //                       pairs[9].value, pairs[14].value);
            //   } else if (strcmp(pairs[0].value, REQUEST_VALUE_REGISTER) == 0)
            //   {
            //     agent_handleRegister(*(con->msgsock), loaded_accounts,
            //                          pairs[3].value, pairs[4].value,
            //                          pairs[8].value);
            //   } else if (strcmp(pairs[0].value, REQUEST_VALUE_TERMHTTP) == 0)
            //   {
            //     agent_handleTermHttp(*(con->msgsock), pairs[7].value);
            //   } else if (strcmp(pairs[0].value, REQUEST_VALUE_LOCK) == 0) {
            //     agent_handleLock(*(con->msgsock), pairs[13].value,
            //                      loaded_accounts, 1);
            //   } else if (strcmp(pairs[0].value, REQUEST_VALUE_UNLOCK) == 0) {
            //     oidc_errno = OIDC_ENOTLOCKED;
            //     server_ipc_writeOidcErrno(*(con->msgsock));
            //   } else {
            //     server_ipc_write(*(con->msgsock), RESPONSE_BADREQUEST,
            //                      "Unknown request type.");
            //   }
            // }
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
