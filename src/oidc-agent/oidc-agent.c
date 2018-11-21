#define _XOPEN_SOURCE 500

#include "oidc-agent.h"
#include "account/account.h"
#include "agent_handler.h"
#include "agent_state.h"
#include "ipc/connection.h"
#include "ipc/ipc.h"
#include "ipc/ipc_async.h"
#include "privileges/agent_privileges.h"
#include "settings.h"
#include "utils/accountUtils.h"
#include "utils/listUtils.h"
#include "utils/memoryCrypt.h"
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

void sig_handler(int signo) {
  switch (signo) {
    case SIGSEGV:
      syslog(LOG_AUTHPRIV | LOG_EMERG, "Caught Signal SIGSEGV");
      break;
    default: syslog(LOG_AUTHPRIV | LOG_EMERG, "Caught Signal %d", signo);
  }
  exit(signo);
}

void daemonize() {
  pid_t pid;
  if ((pid = fork()) == -1) {
    syslog(LOG_AUTHPRIV | LOG_ALERT, "fork %m");
    exit(EXIT_FAILURE);
  } else if (pid > 0) {
    exit(EXIT_SUCCESS);
  }
  if (setsid() < 0) {
    exit(EXIT_FAILURE);
  }
  signal(SIGHUP, SIG_IGN);
  if ((pid = fork()) == -1) {
    syslog(LOG_AUTHPRIV | LOG_ALERT, "fork %m");
    exit(EXIT_FAILURE);
  } else if (pid > 0) {
    printf("%s=%d; export %s;\n", OIDC_PID_ENV_NAME, pid, OIDC_PID_ENV_NAME);
    printf("echo Agent pid $%s\n", OIDC_PID_ENV_NAME);
    exit(EXIT_SUCCESS);
  }
  chdir("/");
  umask(0);
  close(STDIN_FILENO);
  close(STDOUT_FILENO);
  close(STDERR_FILENO);
  open("/dev/null", O_RDONLY);
  open("/dev/null", O_RDWR);
  open("/dev/null", O_RDWR);
}

int main(int argc, char** argv) {
  openlog("oidc-agent", LOG_CONS | LOG_PID, LOG_AUTHPRIV);
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
  initMemoryCrypt();

  if (arguments.kill_flag) {
    char* pidstr = getenv(OIDC_PID_ENV_NAME);
    if (pidstr == NULL) {
      printError("%s not set, cannot kill Agent\n", OIDC_PID_ENV_NAME);
      exit(EXIT_FAILURE);
    }
    pid_t pid = atoi(pidstr);
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

  agent_state.defaultTimeout = arguments.lifetime;

  struct connection* listencon = secAlloc(sizeof(struct connection));
  if (ipc_init(listencon, OIDC_SOCK_ENV_NAME, 1) != OIDC_SUCCESS) {
    printError("%s\n", oidc_serror());
    exit(EXIT_FAILURE);
  }
  if (!arguments.console) {
    daemonize();
  }

  ipc_bindAndListen(listencon);

  list_t* loaded_accounts = list_new();
  loaded_accounts->free   = (void (*)(void*)) & _secFreeAccount;
  loaded_accounts->match  = (int (*)(void*, void*)) & account_matchByName;

  list_t* clientcons = list_new();
  clientcons->free   = (void (*)(void*)) & _secFreeConnection;
  clientcons->match  = (int (*)(void*, void*)) & connection_comparator;

  time_t minDeath = 0;
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
      char* q = ipc_read(*(con->msgsock));
      if (NULL != q) {
        size_t           size = 16;
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
        pairs[11].key = "state";
        pairs[12].key = "lifetime";
        pairs[13].key = "password";
        pairs[14].key = "application_hint";
        pairs[15].key = "subject_token";
        if (getJSONValuesFromString(q, pairs, sizeof(pairs) / sizeof(*pairs)) <
            0) {
          ipc_write(*(con->msgsock), RESPONSE_BADREQUEST, oidc_serror());
        } else {
          if (pairs[0].value) {
            if (strcmp(pairs[0].value, REQUEST_VALUE_CHECK) == 0) {
              ipc_write(*(con->msgsock), RESPONSE_SUCCESS);
            }
            if (agent_state.lock_state.locked) {
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
                                         pairs[6].value, pairs[7].value);
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
                agent_handleTermHttp(*(con->msgsock), pairs[11].value);
              } else if (strcmp(pairs[0].value, REQUEST_VALUE_LOCK) == 0) {
                agent_handleLock(*(con->msgsock), pairs[13].value,
                                 loaded_accounts, 1);
              } else if (strcmp(pairs[0].value, REQUEST_VALUE_TOKENEXCHANGE) ==
                         0) {
                agent_handleTokenExchange(*(con->msgsock), loaded_accounts,
                                          pairs[3].value, pairs[15].value);
              } else if (strcmp(pairs[0].value, REQUEST_VALUE_UNLOCK) == 0) {
                oidc_errno = OIDC_ENOTLOCKED;
                ipc_writeOidcErrno(*(con->msgsock));
              } else {
                ipc_write(*(con->msgsock), RESPONSE_BADREQUEST,
                          "Unknown request type.");
              }
            }
          } else {
            ipc_write(*(con->msgsock), RESPONSE_BADREQUEST, "No request type.");
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
