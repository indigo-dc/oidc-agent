#define _XOPEN_SOURCE 500

#include "oidcp.h"

#include <libgen.h>
#include <signal.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "config_updater.h"
#include "defines/ipc_values.h"
#include "defines/oidc_values.h"
#include "defines/settings.h"
#include "ipc/cryptCommunicator.h"
#include "ipc/pipe.h"
#include "ipc/serveripc.h"
#include "oidc-agent/agent_state.h"
#include "oidc-agent/daemonize.h"
#include "oidc-agent/oidc/device_code.h"
#include "oidc-agent/oidcd/parse_internal.h"
#include "oidc-agent/oidcp/passwords/agent_prompt.h"
#include "oidc-agent/oidcp/passwords/askpass.h"
#include "oidc-agent/oidcp/passwords/password_handler.h"
#include "oidc-agent/oidcp/passwords/password_store.h"
#include "oidc-agent/oidcp/proxy_handler.h"
#include "oidc-agent/oidcp/start_oidcd.h"
#include "utils/agentLogger.h"
#include "utils/crypt/crypt.h"
#include "utils/db/connection_db.h"
#include "utils/disableTracing.h"
#include "utils/json.h"
#include "utils/listUtils.h"
#include "utils/memory.h"
#include "utils/oidc/device.h"
#include "utils/printer.h"
#include "utils/printerUtils.h"
#include "utils/string/stringUtils.h"
#include "utils/uriUtils.h"
#ifdef __MSYS__
#include "utils/registryConnector.h"
#endif

struct connection* unix_listencon;

int main(int argc, char** argv) {
  platform_disable_tracing();
  agent_openlog("oidc-agent.p");
  logger_setloglevel(NOTICE);
  struct arguments arguments;

  /* Set argument defaults */
  initArguments(&arguments);
  srandom(time(NULL));

  argp_parse(&argp, argc, argv, 0, 0, &arguments);
  if (arguments.debug) {
    logger_setloglevel(DEBUG);
  }
  initCrypt();
  if (arguments.kill_flag) {
#ifdef __MSYS__
    char* pidstr = getRegistryValue(OIDC_PID_ENV_NAME);
#else
    char* pidstr = oidc_strcopy(getenv(OIDC_PID_ENV_NAME));
#endif
    if (pidstr == NULL) {
      printError("%s not set, cannot kill Agent\n", OIDC_PID_ENV_NAME);
      exit(EXIT_FAILURE);
    }
    pid_t pid = strToInt(pidstr);
    secFree(pidstr);
    if (0 == pid) {
      printError("%s not set to a valid pid: %s\n", OIDC_PID_ENV_NAME, pidstr);
      exit(EXIT_FAILURE);
    }
    if (kill(pid, SIGTERM) == -1) {
      perror("kill");
      exit(EXIT_FAILURE);
    } else {
#ifdef __MSYS__
      char* oidcSockEnvName = getRegistryValue(OIDC_PID_ENV_NAME);
      unlink(oidcSockEnvName);
      rmdir(dirname(oidcSockEnvName));
      secFree(oidcSockEnvName);
      removeRegistryEntry(OIDC_SOCK_ENV_NAME);
      removeRegistryEntry(OIDC_PID_ENV_NAME);
      printStdout("oidc-agent (Process ID %d) killed\n", pid);
      exit(EXIT_SUCCESS);
#else
      unlink(getenv(OIDC_SOCK_ENV_NAME));
      rmdir(dirname(getenv(OIDC_SOCK_ENV_NAME)));
      printStdout("unset %s;\n", OIDC_SOCK_ENV_NAME);
      printStdout("unset %s;\n", OIDC_PID_ENV_NAME);
      printStdout("echo Agent pid %d killed;\n", pid);
      exit(EXIT_SUCCESS);
#endif
    }
  }
  if (arguments.status) {
    char* res = ipc_cryptCommunicate(
        0, arguments.json ? REQUEST_STATUS_JSON : REQUEST_STATUS);
    if (res == NULL) {
      oidc_perror();
      exit(EXIT_FAILURE);
    }
    char* info = parseForInfo(res);
    if (info == NULL) {
      oidc_perror();
      exit(EXIT_FAILURE);
    }
    printStdout(info);
    secFree(info);
    exit(EXIT_SUCCESS);
  }

  unix_listencon = secAlloc(sizeof(struct connection));
  signal(SIGPIPE, SIG_IGN);
  if (ipc_server_init(unix_listencon, arguments.group, arguments.socket_path) !=
      OIDC_SUCCESS) {
    printError("%s\n", oidc_serror());
    exit(EXIT_FAILURE);
  }

  if (!arguments.console) {
    pid_t daemon_pid = daemonize();
    if (daemon_pid > 0) {
// Export PID of new daemon
#ifdef __MSYS__
      char daemon_pid_string[12];
      sprintf(daemon_pid_string, "%d", daemon_pid);
      createOrUpdateRegistryEntry(OIDC_PID_ENV_NAME, daemon_pid_string);
      createOrUpdateRegistryEntry(OIDC_SOCK_ENV_NAME,
                                  unix_listencon->server->sun_path);
      printStdout("%s=%s\n", OIDC_SOCK_ENV_NAME,
                  unix_listencon->server->sun_path);
      printStdout("%s=%s\n", OIDC_PID_ENV_NAME, daemon_pid_string);
      exit(EXIT_SUCCESS);
#else
      printEnvs(unix_listencon->server->sun_path, daemon_pid, arguments.quiet,
                arguments.json);
      exit(EXIT_SUCCESS);
#endif
    }
  } else {
#ifdef __MSYS__
    char daemon_pid_string[12];
    sprintf(daemon_pid_string, "%d", getpid());
    createOrUpdateRegistryEntry(OIDC_PID_ENV_NAME, daemon_pid_string);
    createOrUpdateRegistryEntry(OIDC_SOCK_ENV_NAME,
                                unix_listencon->server->sun_path);
    printStdout("%s=%s\n", OIDC_SOCK_ENV_NAME,
                unix_listencon->server->sun_path);
    printStdout("%s=%s\n", OIDC_PID_ENV_NAME, daemon_pid_string);
#else
    printEnvs(unix_listencon->server->sun_path, getpid(), arguments.quiet,
              arguments.json);
#endif
  }

  agent_state.defaultTimeout = arguments.lifetime;
  struct ipcPipe pipes       = startOidcd(&arguments);

  if (ipc_bindAndListen(unix_listencon) != 0) {
    exit(EXIT_FAILURE);
  }

  handleClientComm(pipes, &arguments);
}

_Noreturn void handleClientComm(struct ipcPipe          pipes,
                                const struct arguments* arguments) {
  connectionDB_new();
  connectionDB_setFreeFunction((void(*)(void*)) & _secFreeConnection);
  connectionDB_setMatchFunction((matchFunction)connection_comparator);

  time_t minDeath = 0;
  while (1) {
    minDeath               = getMinPasswordDeath();
    struct connection* con = ipc_readAsyncFromMultipleConnectionsWithTimeout(
        *unix_listencon, minDeath);
    if (con == NULL) {  // timeout reached
      removeDeathPasswords();
      continue;
    }
    char* client_req = server_ipc_read(*(con->msgsock));
    if (client_req == NULL) {
      server_ipc_writeOidcErrnoPlain(*(con->msgsock));
    } else {  // NULL != q
      INIT_KEY_VALUE(IPC_KEY_REQUEST, IPC_KEY_PASSWORDENTRY, IPC_KEY_SHORTNAME);
      if (CALL_GETJSONVALUES(client_req) < 0) {
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
          handleOidcdComm(pipes, *(con->msgsock), client_req, arguments);
        } else {  //  no request type
          server_ipc_write(*(con->msgsock), RESPONSE_BADREQUEST,
                           "No request type.");
        }
      }
      SEC_FREE_KEY_VALUES();
      secFree(client_req);
    }
    agent_log(DEBUG, "Remove con from pool");
    connectionDB_removeIfFound(con);
    agent_log(DEBUG, "Currently there are %lu connections",
              connectionDB_getSize());
  }
}

char* _extractShortnameFromReauthenticateInfo(const char* info) {
  const char* const oidcgen = "oidc-gen ";
  const char* const reauth  = " --reauthenticate";
  char*             begin   = strstr(info, oidcgen);
  if (begin == NULL) {
    return NULL;
  }
  begin += strlen(oidcgen);
  char* end = strstr(info, reauth);
  if (end == NULL) {
    return NULL;
  }
  return oidc_strncopy(begin, end - begin);
}

#define SHUTDOWN_IF_D_DIED(res)                                    \
  if (res == NULL) {                                               \
    if (oidc_errno == OIDC_EIPCDIS || oidc_errno == OIDC_EWRITE) { \
      agent_log(ERROR, "oidcd died");                              \
      server_ipc_write(sock, RESPONSE_ERROR, "oidcd died");        \
      exit(EXIT_FAILURE);                                          \
    }                                                              \
    agent_log(ERROR, "no response from oidcd");                    \
    server_ipc_writeOidcErrno(sock);                               \
    return;                                                        \
  }

int _waitForCodeExchangeRequest(time_t expiration, const char* expected_state,
                                struct ipcPipe pipes) {
  while (1) {
    struct connection* con = ipc_readAsyncFromMultipleConnectionsWithTimeout(
        *unix_listencon, expiration);
    if (con == NULL) {  // timeout reached
      removeDeathPasswords();
      return -1;
    }
    char* client_req = server_ipc_read(*(con->msgsock));
    if (client_req == NULL) {
      connectionDB_removeIfFound(con);
      agent_log(DEBUG, "Currently there are %lu connections",
                connectionDB_getSize());
      continue;
    }
    INIT_KEY_VALUE(IPC_KEY_REQUEST, OIDC_KEY_REDIRECTURI);
    if (CALL_GETJSONVALUES(client_req) < 0) {
      server_ipc_writeOidcErrno(*(con->msgsock));
      secFree(client_req);
      SEC_FREE_KEY_VALUES();
      connectionDB_removeIfFound(con);
      agent_log(DEBUG, "Currently there are %lu connections",
                connectionDB_getSize());
      continue;
    }
    KEY_VALUE_VARS(request, uri);
    if (!strcaseequal(_request, REQUEST_VALUE_CODEEXCHANGE)) {
      secFree(client_req);
      SEC_FREE_KEY_VALUES();
      server_ipc_write(
          *(con->msgsock), RESPONSE_ERROR,
          "request currently not acceptable; please try again later");
      connectionDB_removeIfFound(con);
      agent_log(DEBUG, "Currently there are %lu connections",
                connectionDB_getSize());
      continue;
    }
    char* forwarded_res = ipc_communicateThroughPipe(pipes, "%s", client_req);
    secFree(client_req);
    if (forwarded_res == NULL) {
      if (oidc_errno == OIDC_EIPCDIS || oidc_errno == OIDC_EWRITE) {
        agent_log(ERROR, "oidcd died");
        server_ipc_write(*(con->msgsock), RESPONSE_ERROR, "oidcd died");
        exit(EXIT_FAILURE);
      }
      agent_log(ERROR, "no response from oidcd");
      server_ipc_writeOidcErrno(*(con->msgsock));
      SEC_FREE_KEY_VALUES();
      connectionDB_removeIfFound(con);
      agent_log(DEBUG, "Currently there are %lu connections",
                connectionDB_getSize());
      continue;
    }
    server_ipc_write(*(con->msgsock), "%s", forwarded_res);
    secFree(forwarded_res);
    char* state = extractParameterValueFromUri(_uri, "state");
    if (strequal(expected_state, state)) {
      secFree(state);
      SEC_FREE_KEY_VALUES();
      connectionDB_removeIfFound(con);
      agent_log(DEBUG, "Currently there are %lu connections",
                connectionDB_getSize());
      agent_log(DEBUG, "Reaturning");
      return 0;
    }
    secFree(state);
    agent_log(DEBUG, "Once again");
    SEC_FREE_KEY_VALUES();
    connectionDB_removeIfFound(con);
    agent_log(DEBUG, "Currently there are %lu connections",
              connectionDB_getSize());
  }
}

void doReauthenticate(struct ipcPipe pipes, int sock,
                      const char* original_client_req, const char* oidcd_res,
                      const char* info) {
  logger(DEBUG, "Doing automatic reauthentication");
  char* shortname = _extractShortnameFromReauthenticateInfo(info);
  if (shortname == NULL) {
    server_ipc_write(sock, "%s",
                     oidcd_res);  // Forward oidcd response to client
    return;
  }
  logger(DEBUG, "Extracted shortname '%s'", shortname);
  char* reauth_res =
      ipc_communicateThroughPipe(pipes, REQUEST_REAUTHENTICATE, shortname);
  SHUTDOWN_IF_D_DIED(reauth_res);
  INIT_KEY_VALUE(IPC_KEY_DEVICE, IPC_KEY_URI, OIDC_KEY_STATE);
  if (CALL_GETJSONVALUES(reauth_res) < 0) {
    server_ipc_write(sock, "%s", oidcd_res);
    secFree(reauth_res);
    SEC_FREE_KEY_VALUES();
    return;
  }
  secFree(reauth_res);
  KEY_VALUE_VARS(device, url, state);
  if (_url) {
    agent_displayAuthCodeURL(_url, shortname);
    time_t timeout = time(NULL) + AGENT_PROMPT_TIMEOUT;
    if (_waitForCodeExchangeRequest(timeout, _state, pipes)) {
      server_ipc_write(sock, "%s", oidcd_res);
      SEC_FREE_KEY_VALUES();
      return;
    }

    char* lookup_res =
        ipc_communicateThroughPipe(pipes, REQUEST_STATELOOKUP, _state);
    SHUTDOWN_IF_D_DIED(lookup_res);
    char* config = parseStateLookupRes(lookup_res);
    if (config == NULL) {
      server_ipc_write(sock, "%s", oidcd_res);
      SEC_FREE_KEY_VALUES();
      secFree(shortname);
      return;
    }
    SEC_FREE_KEY_VALUES();
    if (writeOIDCFile(config, shortname) != OIDC_SUCCESS) {
      server_ipc_write(sock, "%s", oidcd_res);
      secFree(config);
      secFree(shortname);
      return;
    }
    secFree(shortname);
    secFree(config);

    char* final_res =
        ipc_communicateThroughPipe(pipes, "%s", original_client_req);
    server_ipc_write(sock, "%s", final_res);
    secFree(final_res);
    return;
  }
  if (_device) {
    struct oidc_device_code* dc = getDeviceCodeFromJSON(_device);
    if (dc == NULL) {
      SEC_FREE_KEY_VALUES();
      server_ipc_write(sock, "%s", oidcd_res);
      secFree(shortname);
      return;
    }
    agent_displayDeviceCode(dc, shortname);
    if (dc->expires_in > AGENT_PROMPT_TIMEOUT) {
      dc->expires_in = AGENT_PROMPT_TIMEOUT;
    }
    char* config = agent_pollDeviceCode(
        _device, dc->interval, dc->expires_in ? time(NULL) + dc->expires_in : 0,
        0, &pipes);
    SEC_FREE_KEY_VALUES();
    if (config == NULL) {
      server_ipc_write(sock, "%s", oidcd_res);
      secFree(shortname);
      return;
    }
    if (writeOIDCFile(config, shortname) != OIDC_SUCCESS) {
      server_ipc_write(sock, "%s", oidcd_res);
      secFree(config);
      secFree(shortname);
      return;
    }
    secFree(config);
    secFree(shortname);

    char* final_res =
        ipc_communicateThroughPipe(pipes, "%s", original_client_req);
    server_ipc_write(sock, "%s", final_res);
    secFree(final_res);
    return;
  }
  SEC_FREE_KEY_VALUES();
  server_ipc_write(sock, "%s", oidcd_res);
  secFree(shortname);
  return;
}

void handleOidcdComm(struct ipcPipe pipes, int sock, const char* msg,
                     const struct arguments* arguments) {
  char* send = oidc_strcopy(msg);
  INIT_KEY_VALUE(IPC_KEY_REQUEST, OIDC_KEY_REFRESHTOKEN, IPC_KEY_SHORTNAME,
                 IPC_KEY_APPLICATIONHINT, IPC_KEY_ISSUERURL, OIDC_KEY_ERROR,
                 IPC_KEY_INFO);
  while (1) {
    // RESET_KEY_VALUE_VALUES_TO_NULL();
    char* oidcd_res = ipc_communicateThroughPipe(pipes, "%s", send);
    secFree(send);
    SHUTDOWN_IF_D_DIED(oidcd_res);
    // check response, it might be an internal request
    if (CALL_GETJSONVALUES(oidcd_res) < 0) {
      server_ipc_write(sock, RESPONSE_BADREQUEST, oidc_serror());
      secFree(oidcd_res);
      SEC_FREE_KEY_VALUES();
      return;
    }
    KEY_VALUE_VARS(request, refresh_token, shortname, application_hint, issuer,
                   error, info);
    if (_request == NULL) {  // if the response is the final response, forward
                             // it to the client
      if (_error != NULL && _info != NULL &&
          (strstarts(_error, "invalid_grant:") ||
           strstarts(_error, "invalid_token:") ||
           errorMessageIsForError(_error, OIDC_ENOREFRSH)) &&
          strSubString(_info, "--reauthenticate") &&
          !arguments->no_autoreauthenticate) {
        doReauthenticate(pipes, sock, msg, oidcd_res, _info);
        SEC_FREE_KEY_VALUES();
        secFree(oidcd_res);
        return;
      }
      server_ipc_write(sock, "%s",
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
      send           = e == OIDC_SUCCESS ? oidc_strcopy(RESPONSE_SUCCESS)
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
      secFree(account);
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
