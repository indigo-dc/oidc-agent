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
#include "oidc-agent/oidcp/passwords/askpass.h"
#include "oidc-agent/oidcp/passwords/password_handler.h"
#include "oidc-agent/oidcp/passwords/password_store.h"
#include "oidc-agent/oidcp/proxy_handler.h"
#ifndef __APPLE__
#include "privileges/agent_privileges.h"
#endif
#include "utils/agentLogger.h"
#include "utils/crypt/crypt.h"
#include "utils/db/connection_db.h"
#include "utils/disableTracing.h"
#include "utils/json.h"
#include "utils/listUtils.h"
#include "utils/memory.h"
#include "utils/printer.h"
#include "utils/stringUtils.h"

#include <libgen.h>
#include <signal.h>
#ifndef __APPLE__
#include <sys/prctl.h>
#endif
#include <time.h>
#include <unistd.h>

struct ipcPipe startOidcd(const struct arguments* arguments) {
  struct pipeSet pipes = ipc_pipe_init();
  if (pipes.pipe1.rx == -1) {
    agent_log(ERROR, "could not create pipes");
    exit(EXIT_FAILURE);
  }
  pid_t ppid_before_fork = getpid();
  pid_t pid              = fork();
  if (pid == -1) {
    agent_log(ERROR, "fork %m");
    exit(EXIT_FAILURE);
  }
  if (pid == 0) {  // child
#ifndef __APPLE__
    // init child so that it exists if parent (oidcp) is killed.
    int r = prctl(PR_SET_PDEATHSIG, SIGTERM);
    if (r == -1) {
      agent_log(ERROR, "prctl %m");
      exit(EXIT_FAILURE);
    }
#endif
    // test in case the original parent exited just before the prctl() call
    if (getppid() != ppid_before_fork) {
      agent_log(ERROR, "Parent died shortly after fork");
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
  agent_openlog("oidc-agent.p");
  logger_setloglevel(NOTICE);
  struct arguments arguments;

  /* Set argument defaults */
  initArguments(&arguments);
  srandom(time(NULL));

  argp_parse(&argp, argc, argv, 0, 0, &arguments);
  agent_log(DEBUG, "%d", __LINE__);
  if (arguments.debug) {
    logger_setloglevel(DEBUG);
  }
  agent_log(DEBUG, "%d", __LINE__);
#ifndef __APPLE__
  if (arguments.seccomp) {
    initOidcAgentPrivileges(&arguments);
  }
#endif
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
      printStdout("unset %s;\n", OIDC_SOCK_ENV_NAME);
      printStdout("unset %s;\n", OIDC_PID_ENV_NAME);
      printStdout("echo Agent pid %d killed;\n", pid);
      exit(EXIT_SUCCESS);
    }
  }

  struct connection* listencon = secAlloc(sizeof(struct connection));
  signal(SIGPIPE, SIG_IGN);
  if (ipc_server_init(listencon, OIDC_SOCK_ENV_NAME, arguments.group) !=
      OIDC_SUCCESS) {
    printError("%s\n", oidc_serror());
    exit(EXIT_FAILURE);
  }
  if (!arguments.console) {
    daemonize();
  }

  agent_state.defaultTimeout = arguments.lifetime;
  struct ipcPipe pipes       = startOidcd(&arguments);

  ipc_bindAndListen(listencon);

  handleClientComm(listencon, pipes, &arguments);

  return EXIT_FAILURE;
}

void handleClientComm(struct connection* listencon, struct ipcPipe pipes,
                      const struct arguments* arguments) {
  connectionDB_new();
  connectionDB_setFreeFunction((void (*)(void*)) & _secFreeConnection);
  connectionDB_setMatchFunction((matchFunction)connection_comparator);

  time_t minDeath = 0;
  while (1) {
    minDeath = getMinPasswordDeath();
    struct connection* con =
        ipc_readAsyncFromMultipleConnectionsWithTimeout(*listencon, minDeath);
    if (con == NULL) {  // timeout reached
      removeDeathPasswords();
      continue;
    }
    char* q = server_ipc_read(*(con->msgsock));
    if (q == NULL) {
      server_ipc_writeOidcErrnoPlain(*(con->msgsock));
    } else {  // NULL != q
      INIT_KEY_VALUE(IPC_KEY_REQUEST, IPC_KEY_PASSWORDENTRY, IPC_KEY_SHORTNAME);
      if (CALL_GETJSONVALUES(q) < 0) {
        server_ipc_write(*(con->msgsock), RESPONSE_BADREQUEST, oidc_serror());
      } else {
        KEY_VALUE_VARS(request, passwordentry, shortname);
        if (_request) {
          if (strequal(_request, REQUEST_VALUE_ADD) ||
              strequal(_request, REQUEST_VALUE_GEN)) {
            pw_handleSave(_passwordentry, arguments->pw_lifetime);
          } else if (strequal(_request, REQUEST_VALUE_REMOVE)) {
            removePasswordFor(_shortname);
          } else if (strequal(_request, REQUEST_VALUE_REMOVEALL)) {
            removeAllPasswords();
          }
          handleOidcdComm(pipes, *(con->msgsock), q);
        } else {  //  no request type
          server_ipc_write(*(con->msgsock), RESPONSE_BADREQUEST,
                           "No request type.");
        }
      }
      SEC_FREE_KEY_VALUES();
      secFree(q);
    }
    agent_log(DEBUG, "Remove con from pool");
    connectionDB_removeIfFound(con);
    agent_log(DEBUG, "Currently there are %lu connections",
              connectionDB_getSize());
  }
}

void handleOidcdComm(struct ipcPipe pipes, int sock, const char* msg) {
  char* send = oidc_strcopy(msg);
  INIT_KEY_VALUE(IPC_KEY_REQUEST, OIDC_KEY_REFRESHTOKEN, IPC_KEY_SHORTNAME,
                 IPC_KEY_APPLICATIONHINT, IPC_KEY_ISSUERURL);
  while (1) {
    // RESET_KEY_VALUE_VALUES_TO_NULL();
    char* oidcd_res = ipc_communicateThroughPipe(pipes, send);
    secFree(send);
    if (oidcd_res == NULL) {
      if (oidc_errno == OIDC_EIPCDIS || oidc_errno == OIDC_EWRITE) {
        agent_log(ERROR, "oidcd died");
        server_ipc_write(sock, RESPONSE_ERROR, "oidcd died");
        exit(EXIT_FAILURE);
      }
      agent_log(ERROR, "no response from oidcd");
      server_ipc_writeOidcErrno(sock);
      return;
    }  // oidcd_res!=NULL
       // check response, it might be an internal request
    if (CALL_GETJSONVALUES(oidcd_res) < 0) {
      server_ipc_write(sock, RESPONSE_BADREQUEST, oidc_serror());
      secFree(oidcd_res);
      SEC_FREE_KEY_VALUES();
      return;
    }
    KEY_VALUE_VARS(request, refresh_token, shortname, application_hint, issuer);
    if (_request == NULL) {  // if the response is the final response, forward
                             // it to the client
      server_ipc_write(sock,
                       oidcd_res);  // Forward oidcd response to client
      secFree(oidcd_res);
      SEC_FREE_KEY_VALUES();
      return;
    }
    secFree(oidcd_res);
    if (strequal(_request, INT_REQUEST_VALUE_UPD_REFRESH)) {
      oidc_error_t e = updateRefreshToken(_shortname, _refresh_token);
      send           = e == OIDC_SUCCESS ? oidc_strcopy(RESPONSE_SUCCESS)
                               : oidc_sprintf(RESPONSE_ERROR, oidc_serror());
      SEC_FREE_KEY_VALUES();
      continue;
    } else if (strequal(_request, INT_REQUEST_VALUE_AUTOLOAD)) {
      char* config = getAutoloadConfig(_shortname, _issuer, _application_hint);
      send         = config
                 ? oidc_sprintf(RESPONSE_STATUS_CONFIG, STATUS_SUCCESS, config)
                 : oidc_sprintf(INT_RESPONSE_ERROR, oidc_errno);
      secFree(config);
      SEC_FREE_KEY_VALUES();
      continue;
    } else if (strequal(_request, INT_REQUEST_VALUE_CONFIRM)) {
      oidc_error_t e =
          _issuer ? askpass_getConfirmationWithIssuer(_issuer, _shortname,
                                                      _application_hint)
                  : askpass_getConfirmation(_shortname, _application_hint);
      send = e == OIDC_SUCCESS ? oidc_strcopy(RESPONSE_SUCCESS)
                               : oidc_sprintf(INT_RESPONSE_ERROR, oidc_errno);
      SEC_FREE_KEY_VALUES();
      continue;
    } else if (strequal(_request, INT_REQUEST_VALUE_CONFIRMIDTOKEN)) {
      oidc_error_t e = _issuer ? askpass_getIdTokenConfirmationWithIssuer(
                                     _issuer, _shortname, _application_hint)
                               : askpass_getIdTokenConfirmation(
                                     _shortname, _application_hint);
      send = e == OIDC_SUCCESS ? oidc_strcopy(RESPONSE_SUCCESS)
                               : oidc_sprintf(INT_RESPONSE_ERROR, oidc_errno);
      SEC_FREE_KEY_VALUES();
      continue;
    } else if (strequal(_request, INT_REQUEST_VALUE_QUERY_ACCDEFAULT)) {
      char* account = NULL;
      if (strValid(_issuer)) {  // default for this issuer
        account = getDefaultAccountConfigForIssuer(_issuer);
      } else {                      // global default
        oidc_errno = OIDC_NOTIMPL;  // TODO
      }
      send = oidc_sprintf(INT_RESPONSE_ACCDEFAULT, account ?: "");
      SEC_FREE_KEY_VALUES();
      continue;
    } else {
      server_ipc_write(
          sock, "Internal communication error: unknown internal request");
      SEC_FREE_KEY_VALUES();
      return;
    }
  }
}
