#define _XOPEN_SOURCE 500

#include "oidcp.h"
#include "defines/ipc_values.h"
#include "defines/oidc_values.h"
#include "defines/settings.h"
#include "ipc/cryptIpc.h"
#include "ipc/pipe.h"
#include "ipc/serveripc.h"
#include "list/list.h"
#include "oidc-agent/agent_state.h"
#include "oidc-agent/daemonize.h"
#include "oidc-agent/oidcd/oidcd.h"
#include "oidc-agent/oidcp/passwords/password_handler.h"
#include "oidc-agent/oidcp/passwords/password_store.h"
#include "oidc-agent/oidcp/proxy_handler.h"
#include "privileges/agent_privileges.h"
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
    // init child so that it exists if parent (oidcp) is killed.
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

  agent_state.defaultTimeout = arguments.lifetime;
  struct ipcPipe pipes       = startOidcd(&arguments);

  ipc_bindAndListen(listencon);

  handleClientComm(listencon, pipes);

  return EXIT_FAILURE;
}

void handleClientComm(struct connection* listencon, struct ipcPipe pipes) {
  list_t* clientcons = list_new();
  clientcons->free   = (void (*)(void*)) & _secFreeConnection;
  clientcons->match  = (int (*)(void*, void*)) & connection_comparator;

  time_t minDeath = 0;
  while (1) {
    minDeath               = getMinPasswordDeath();
    struct connection* con = ipc_readAsyncFromMultipleConnectionsWithTimeout(
        *listencon, clientcons, minDeath);
    if (con == NULL) {  // timeout reached
      removeDeathPasswords();
      continue;
    }
    char* q = server_ipc_read(*(con->msgsock));
    if (q == NULL) {
      server_ipc_writeOidcErrnoPlain(*(con->msgsock));
    } else {  // NULL != q
      size_t           size = 3;
      struct key_value pairs[size];
      for (size_t i = 0; i < size; i++) { pairs[i].value = NULL; }
      pairs[0].key = IPC_KEY_REQUEST;
      pairs[1].key = IPC_KEY_PASSWORDENTRY;
      pairs[2].key = IPC_KEY_SHORTNAME;
      if (getJSONValuesFromString(q, pairs, sizeof(pairs) / sizeof(*pairs)) <
          0) {
        server_ipc_write(*(con->msgsock), RESPONSE_BADREQUEST, oidc_serror());
      } else {
        const char* request = pairs[0].value;
        if (request) {
          if (strequal(request, REQUEST_VALUE_ADD) ||
              strequal(request, REQUEST_VALUE_GEN)) {
            pw_handleSave(pairs[1].value);
          } else if (strequal(request, REQUEST_VALUE_REMOVE)) {
            removePasswordFor(pairs[2].value);
          } else if (strequal(request, REQUEST_VALUE_REMOVEALL)) {
            removeAllPasswords();
          }
          handleOidcdComm(pipes, *(con->msgsock), q);
        } else {  // pairs[0].value NULL - no request type
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

void handleOidcdComm(struct ipcPipe pipes, int sock, const char* msg) {
  char*            send = oidc_strcopy(msg);
  size_t           size = 3;
  struct key_value pairs[size];
  pairs[0].key = IPC_KEY_REQUEST;
  pairs[1].key = OIDC_KEY_REFRESHTOKEN;
  pairs[2].key = IPC_KEY_SHORTNAME;
  while (1) {
    for (size_t i = 0; i < size; i++) { pairs[i].value = NULL; }
    char* oidcd_res = ipc_communicateThroughPipe(pipes, send);
    secFree(send);
    if (oidcd_res == NULL) {
      if (oidc_errno == OIDC_EIPCDIS) {
        syslog(LOG_AUTHPRIV | LOG_ERR, "oidcd died");
        exit(EXIT_FAILURE);
      }
      syslog(LOG_AUTHPRIV | LOG_ERR, "no response from oidcd");
      server_ipc_writeOidcErrno(sock);
      return;
    }  // oidcd_res!=NULL
       // check response, it might be an internal request
    if (getJSONValuesFromString(oidcd_res, pairs,
                                sizeof(pairs) / sizeof(*pairs)) < 0) {
      server_ipc_write(sock, RESPONSE_BADREQUEST, oidc_serror());
      secFree(oidcd_res);
      secFreeKeyValuePairs(pairs, sizeof(pairs) / sizeof(*pairs));
      return;
    }
    char* request = pairs[0].value;
    if (!request) {  // if the response is the final response, forward it to the
                     // client
      server_ipc_write(sock,
                       oidcd_res);  // Forward oidcd response to client
      secFree(oidcd_res);
      secFreeKeyValuePairs(pairs, sizeof(pairs) / sizeof(*pairs));
      return;
    }
    secFree(oidcd_res);
    if (strequal(request, INT_REQUEST_VALUE_UPD_REFRESH)) {
      oidc_error_t e = updateRefreshToken(pairs[2].value, pairs[1].value);
      if (e == OIDC_SUCCESS) {
        send = oidc_strcopy(RESPONSE_SUCCESS);
      } else {
        send = oidc_sprintf(RESPONSE_ERROR, oidc_serror());
      }
      secFreeKeyValuePairs(pairs, sizeof(pairs) / sizeof(*pairs));
      continue;
    } else {
      server_ipc_write(
          sock, "Internal communication error: unknown internal request");
      secFreeKeyValuePairs(pairs, sizeof(pairs) / sizeof(*pairs));
      return;
    }
  }
}
