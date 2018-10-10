#define _XOPEN_SOURCE 500

#include "oidc-agent.h"
#include "account.h"
#include "agent_handler.h"
#include "ipc/connection.h"
#include "ipc/ipc.h"
#include "ipc/ipc_async.h"
#include "oidc_error.h"
#include "settings.h"

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
  arguments.kill_flag = 0;
  arguments.console   = 0;
  arguments.debug     = 0;
  srandom(time(NULL));

  argp_parse(&argp, argc, argv, 0, 0, &arguments);
  if (arguments.debug) {
    setlogmask(LOG_UPTO(LOG_DEBUG));
  }

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

  // signal(SIGSEGV, sig_handler);

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
  loaded_accounts->free   = (void (*)(void*)) & clearFreeAccount;
  loaded_accounts->match  = (int (*)(void*, void*)) & account_matchByName;

  list_t* clientcons = list_new();
  clientcons->free   = (void (*)(void*)) & clearFreeConnection;
  clientcons->match  = (int (*)(void*, void*)) & connection_comparator;

  while (1) {
    struct connection* con = ipc_async(*listencon, clientcons);
    if (con == NULL) {
      // should never happen
      syslog(LOG_AUTHPRIV | LOG_ALERT, "Something went wrong");
      exit(EXIT_FAILURE);
    } else {
      char* q = ipc_read(*(con->msgsock));
      if (NULL != q) {
        struct key_value pairs[12];
        pairs[0].key    = "request";
        pairs[0].value  = NULL;
        pairs[1].key    = "account";
        pairs[1].value  = NULL;
        pairs[2].key    = "min_valid_period";
        pairs[2].value  = NULL;
        pairs[3].key    = "config";
        pairs[3].value  = NULL;
        pairs[4].key    = "flow";
        pairs[4].value  = NULL;
        pairs[5].key    = "code";
        pairs[5].value  = NULL;
        pairs[6].key    = "redirect_uri";
        pairs[6].value  = NULL;
        pairs[7].key    = "state";
        pairs[7].value  = NULL;
        pairs[8].key    = "authorization";
        pairs[8].value  = NULL;
        pairs[9].key    = "scope";
        pairs[9].value  = NULL;
        pairs[10].key   = "oidc_device";
        pairs[10].value = NULL;
        pairs[11].key   = "state";
        pairs[11].value = NULL;
        if (getJSONValues(q, pairs, sizeof(pairs) / sizeof(*pairs)) < 0) {
          ipc_write(*(con->msgsock), RESPONSE_BADREQUEST, oidc_serror());
        } else {
          if (pairs[0].value) {
            if (strcmp(pairs[0].value, REQUEST_VALUE_GEN) == 0) {
              agent_handleGen(*(con->msgsock), loaded_accounts, pairs[3].value,
                              pairs[4].value);
            } else if (strcmp(pairs[0].value, REQUEST_VALUE_CODEEXCHANGE) ==
                       0) {
              agent_handleCodeExchange(*(con->msgsock), loaded_accounts,
                                       pairs[3].value, pairs[5].value,
                                       pairs[6].value, pairs[7].value);
            } else if (strcmp(pairs[0].value, REQUEST_VALUE_STATELOOKUP) == 0) {
              agent_handleStateLookUp(*(con->msgsock), loaded_accounts,
                                      pairs[7].value);
            } else if (strcmp(pairs[0].value, REQUEST_VALUE_DEVICELOOKUP) ==
                       0) {
              agent_handleDeviceLookup(*(con->msgsock), loaded_accounts,
                                       pairs[3].value, pairs[10].value);
            } else if (strcmp(pairs[0].value, REQUEST_VALUE_ADD) == 0) {
              agent_handleAdd(*(con->msgsock), loaded_accounts, pairs[3].value);
            } else if (strcmp(pairs[0].value, REQUEST_VALUE_REMOVE) == 0) {
              agent_handleRm(*(con->msgsock), loaded_accounts, pairs[3].value,
                             0);
            } else if (strcmp(pairs[0].value, REQUEST_VALUE_DELETE) == 0) {
              agent_handleRm(*(con->msgsock), loaded_accounts, pairs[3].value,
                             1);
            } else if (strcmp(pairs[0].value, REQUEST_VALUE_ACCESSTOKEN) == 0) {
              agent_handleToken(*(con->msgsock), loaded_accounts,
                                pairs[1].value, pairs[2].value, pairs[9].value);
            } else if (strcmp(pairs[0].value, REQUEST_VALUE_ACCOUNTLIST) == 0) {
              agent_handleList(*(con->msgsock), loaded_accounts);
            } else if (strcmp(pairs[0].value, REQUEST_VALUE_REGISTER) == 0) {
              agent_handleRegister(*(con->msgsock), loaded_accounts,
                                   pairs[3].value, pairs[8].value);
            } else if (strcmp(pairs[0].value, REQUEST_VALUE_TERMHTTP) == 0) {
              agent_handleTermHttp(*(con->msgsock), pairs[11].value);
            } else {
              ipc_write(*(con->msgsock), RESPONSE_BADREQUEST,
                        "Unknown request type.");
            }
          } else {
            ipc_write(*(con->msgsock), RESPONSE_BADREQUEST, "No request type.");
          }
        }
        clearFreeKeyValuePairs(pairs, sizeof(pairs) / sizeof(*pairs));
        secFree(q);
      }
      syslog(LOG_AUTHPRIV | LOG_DEBUG, "Remove con from pool");
      list_remove(clientcons, list_find(clientcons, con));
      syslog(LOG_AUTHPRIV | LOG_DEBUG, "Currently there are %d connections",
             clientcons->len);
    }
  }
  return EXIT_FAILURE;
}
