#define _XOPEN_SOURCE 500
#define _POSIX_C_SOURCE 200112L

#include "oidcp.h"

#include <libgen.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "account/account.h"
#include "account/issuer_helper.h"
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
#include "oidc-agent/stats/statlogger.h"
#include "oidc-gen/promptAndSet/name.h"
#include "utils/agentLogger.h"
#include "utils/config/gen_config.h"
#include "utils/config/issuerConfig.h"
#include "utils/crypt/crypt.h"
#include "utils/db/connection_db.h"
#include "utils/disableTracing.h"
#include "utils/json.h"
#include "utils/listUtils.h"
#include "utils/memory.h"
#include "utils/oidc/device.h"
#include "utils/printer.h"
#include "utils/printerUtils.h"
#include "utils/prompt_mode.h"
#include "utils/string/stringUtils.h"
#include "utils/uriUtils.h"
#ifdef __MSYS__
#include "utils/registryConnector.h"
#endif

static void handleAccountInfo(struct ipcPipe pipes, int sock) {
  char* loaded_res = ipc_communicateThroughPipe(pipes, REQUEST_LOADEDACCOUNTS);
  char* info       = parseForInfo(loaded_res);
  list_t* loaded   = JSONArrayStringToList(info);
  secFree(info);
  char* accountsInfo = getAccountInfos(loaded);
  secFreeList(loaded);
  server_ipc_write(sock, RESPONSE_SUCCESS_INFO_OBJECT, accountsInfo);
  secFree(accountsInfo);
}

static struct connection* unix_listencon;

static pid_t parent_pid = -1;

static void check_parent_alive(void) {
  if (parent_pid != -1 && getppid() != parent_pid) {
    exit(EXIT_SUCCESS);
  }
}

_Noreturn static void handleClientComm(struct ipcPipe          pipes,
                                       const struct arguments* arguments,
                                       time_t parent_alive_interval) {
  connectionDB_new();
  connectionDB_setFreeFunction((void (*)(void*)) & _secFreeConnection);
  connectionDB_setMatchFunction((matchFunction)connection_comparator);

  time_t deadline = 0;
  while (1) {
    deadline = getMinPasswordDeath();
    if (parent_alive_interval > 0) {
      time_t parent_check = time(NULL) + parent_alive_interval;
      if (deadline == 0 || deadline > parent_check) {
        deadline = parent_check;
      }
    }
    struct connection* con = ipc_readAsyncFromMultipleConnectionsWithTimeout(
        *unix_listencon, deadline);
    if (con == NULL) {  // timeout reached
      if (parent_alive_interval != 0) {
        check_parent_alive();
      }
      removeDeathPasswords();
      continue;
    }
    char* client_req = server_ipc_read(*(con->msgsock));
    if (client_req == NULL) {
      server_ipc_writeOidcErrnoPlain(*(con->msgsock));
    } else {
      statlog(client_req);
      INIT_KEY_VALUE(IPC_KEY_REQUEST, IPC_KEY_PASSWORDENTRY, IPC_KEY_SHORTNAME);
      if (CALL_GETJSONVALUES(client_req) < 0) {
        server_ipc_write(*(con->msgsock), RESPONSE_BADREQUEST, oidc_serror());
      } else {
        KEY_VALUE_VARS(request, passwordentry, shortname);
        if (_request) {
          unsigned char skipOIDCDComm = 0;
          if (strequal(_request, REQUEST_VALUE_ADD) ||
              strequal(_request, REQUEST_VALUE_GEN)) {
            pw_handleSave(_passwordentry);
          } else if (strequal(_request, REQUEST_VALUE_REMOVE)) {
            removePasswordFor(_shortname);
          } else if (strequal(_request, REQUEST_VALUE_REMOVEALL)) {
            removeAllPasswords();
          } else if (strequal(_request, REQUEST_VALUE_ACCOUNTINFO)) {
            handleAccountInfo(pipes, *(con->msgsock));
            skipOIDCDComm = 1;
          }
          if (!skipOIDCDComm) {
            handleOidcdComm(pipes, *(con->msgsock), client_req, arguments);
          }
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
    oidc_perror();
    exit(EXIT_FAILURE);
  }

  time_t parent_alive_interval = 0;
  if (!arguments.console) {
    unsigned char commandSet = strValid(arguments.command);
    if (commandSet) {
      parent_alive_interval = 10;
    }
    pid_t daemon_pid = daemonize(!commandSet);
    if (daemon_pid > 0) {
      // Export PID of new daemon
      if (commandSet) {
        char* pid_str = oidc_sprintf("%d", daemon_pid);
        if (setenv(OIDC_SOCK_ENV_NAME, unix_listencon->server->sun_path, 1) ==
                -1 ||
            setenv(OIDC_PID_ENV_NAME, pid_str, 1) == -1) {
          secFree(pid_str);
          oidc_setErrnoError();
          oidc_perror();
          exit(oidc_errno);
        }
        secFree(pid_str);
        execvp(arguments.command, arguments.args);
        oidc_setErrnoError();
        oidc_perror();
        exit(oidc_errno);
      }
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

  parent_pid = getppid();

  agent_state.defaultTimeout = arguments.lifetime;
  struct ipcPipe pipes       = startOidcd(&arguments);

  if (ipc_bindAndListen(unix_listencon, arguments.group) != 0) {
    exit(EXIT_FAILURE);
  }

  handleClientComm(pipes, &arguments, parent_alive_interval);
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
      time_t remaining_time = expiration - time(NULL);
      char*  error_msg      = oidc_sprintf(
          "request currently not acceptable; please try again later (%ds)",
          remaining_time);
      server_ipc_write(*(con->msgsock), RESPONSE_ERROR, error_msg);
      secFree(error_msg);
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
      agent_log(DEBUG, "Returning");
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

const char* _getMytokenURLToUse(struct ipcPipe pipes, const char* issuer) {
  unsigned char useMytokenServer = 0;
  if (!getGenConfig()->prefer_mytoken_over_oidc ||
      getGenConfig()->default_mytoken_server == NULL) {
    return NULL;
  }
  char* providers_res = ipc_communicateThroughPipe(
      pipes, REQUEST_MYTOKEN_PROVIDERS, getGenConfig()->default_mytoken_server,
      "", "");
  if (providers_res == NULL) {
    return NULL;
  }
  char* providers = getJSONValueFromString(providers_res, IPC_KEY_INFO);
  secFree(providers_res);
  if (providers == NULL) {
    return NULL;
  }
  list_t* providers_l = JSONArrayStringToList(providers);
  secFree(providers);
  if (providers_l == NULL) {
    return NULL;
  }
  list_node_t*     node;
  list_iterator_t* it = list_iterator_new(providers_l, LIST_HEAD);
  while ((node = list_iterator_next(it))) {
    char* p   = node->val;
    char* iss = getJSONValueFromString(p, OIDC_KEY_ISSUER);
    if (compIssuerUrls(issuer, iss)) {
      useMytokenServer = 1;
      break;
    }
  }
  list_iterator_destroy(it);
  secFreeList(providers_l);
  return useMytokenServer ? getGenConfig()->default_mytoken_server : NULL;
}

void _parseInternalGen(struct ipcPipe pipes, int sock, char* res,
                       const char* original_client_req,
                       const char* error_res_fmt, const char* error_res_arg,
                       const char* shortname, unsigned char reauth_intro) {
  SHUTDOWN_IF_D_DIED(res);
  INIT_KEY_VALUE(IPC_KEY_DEVICE, IPC_KEY_URI, OIDC_KEY_STATE, IPC_KEY_REQUEST,
                 INT_IPC_KEY_ACTION, IPC_KEY_ISSUERURL, IPC_KEY_SHORTNAME);
  if (CALL_GETJSONVALUES(res) < 0) {
    server_ipc_write(sock, error_res_fmt, error_res_arg);
    secFree(res);
    SEC_FREE_KEY_VALUES();
    return;
  }
  secFree(res);
  KEY_VALUE_VARS(device, url, state, request, action, issuer, shortname);
  if (_url) {
    agent_displayAuthCodeURL(_url, shortname, reauth_intro);
    time_t timeout = time(NULL) + AGENT_PROMPT_TIMEOUT;
    if (_waitForCodeExchangeRequest(timeout, _state, pipes)) {
      server_ipc_write(sock, error_res_fmt, error_res_arg);
      SEC_FREE_KEY_VALUES();
      return;
    }

    char* lookup_res =
        ipc_communicateThroughPipe(pipes, REQUEST_STATELOOKUP, _state);
    SHUTDOWN_IF_D_DIED(lookup_res);
    char* config = parseStateLookupRes(lookup_res, pipes);
    if (config == NULL) {
      server_ipc_write(sock, error_res_fmt, error_res_arg);
      SEC_FREE_KEY_VALUES();
      return;
    }
    SEC_FREE_KEY_VALUES();
    if (writeOIDCFile(config, shortname) != OIDC_SUCCESS) {
      server_ipc_write(sock, error_res_fmt, error_res_arg);
      secFree(config);
      return;
    }
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
      server_ipc_write(sock, error_res_fmt, error_res_arg);
      return;
    }
    agent_displayDeviceCode(dc, shortname, reauth_intro);
    if (dc->expires_in > AGENT_PROMPT_TIMEOUT) {
      dc->expires_in = AGENT_PROMPT_TIMEOUT;
    }
    char* config = agent_pollDeviceCode(
        _device, dc->interval, dc->expires_in ? time(NULL) + dc->expires_in : 0,
        0, &pipes);
    SEC_FREE_KEY_VALUES();
    if (config == NULL) {
      server_ipc_write(sock, error_res_fmt, error_res_arg);
      return;
    }
    if (writeOIDCFile(config, shortname) != OIDC_SUCCESS) {
      secFree(config);
      server_ipc_write(sock, error_res_fmt, error_res_arg);
      return;
    }
    secFree(config);

    char* final_res =
        ipc_communicateThroughPipe(pipes, "%s", original_client_req);
    server_ipc_write(sock, "%s", final_res);
    secFree(final_res);
    return;
  }
  if (strcaseequal(_request, INT_REQUEST_VALUE_UPD_ISSUER)) {
    oidcp_updateIssuerConfig(_action, _issuer, _shortname);
    SEC_FREE_KEY_VALUES();
    return _parseInternalGen(
        pipes, sock, ipc_communicateThroughPipe(pipes, RESPONSE_SUCCESS),
        original_client_req, error_res_fmt, error_res_arg, shortname,
        reauth_intro);
  }
  SEC_FREE_KEY_VALUES();
  server_ipc_write(sock, error_res_fmt, error_res_arg);
}

void handleAutoGen(struct ipcPipe pipes, int sock,
                   const char* original_client_req, const char* issuer,
                   const char* scopes, const char* application_hint) {
  struct oidc_account* account = secAlloc(sizeof(struct oidc_account));
  set_prompt_mode(PROMPT_MODE_GUI);
  set_pw_prompt_mode(PROMPT_MODE_GUI);
  account_setIssuerUrl(account, oidc_strcopy(issuer));
  account_setMytokenUrl(account,
                        oidc_strcopy(_getMytokenURLToUse(pipes, issuer)));
  const list_t* flows          = NULL;
  unsigned char usedUserClient = 0;
  if (account_getMytokenUrl(account)) {
    if (getGenConfig()->default_mytoken_profile) {
      account_setUsedMytokenProfile(
          account,
          oidc_sprintf("\"%s\"", getGenConfig()->default_mytoken_profile));
    }
  } else {
    const struct issuerConfig* issConfig = getIssuerConfig(issuer);
    if (issConfig && issConfig->user_client &&
        issConfig->user_client->client_id) {
      usedUserClient = 1;
      updateAccountWithUserClientInfo(account);
      flows = getUserClientFlows(account_getIssuerUrl(account));
    } else {
      updateAccountWithPublicClientInfo(account);
      flows = getPubClientFlows(account_getIssuerUrl(account));
    }
    if (!strValid(account_getClientId(account))) {
      secFreeAccount(account);
      server_ipc_write(sock, RESPONSE_ERROR, ACCOUNT_NOT_LOADED);
      return;
    }
  }
  if (strequal(scopes, AGENT_SCOPE_ALL)) {
    if (account_getMytokenUrl(account)) {
      account_setScopeExact(account, oidc_strcopy(scopes));
    } else {
      account_setScope(account, usedUserClient
                                    ? getScopesForUserClient(account)
                                    : getScopesForPublicClient(account));
    }
  } else {
    account_setScope(account, oidc_strcopy(scopes));
  }

  agent_log(DEBUG, "Prompting user for confirmation for autogen for '%s'",
            issuer);
  char* application_str =
      strValid(application_hint) ? oidc_sprintf("%s ", application_hint) : NULL;
  char* prompt_text = gettext("link-identity", application_str, issuer);
  secFree(application_str);
  if (!agent_promptConsentDefaultYes(prompt_text)) {
    secFree(prompt_text);
    agent_log(DEBUG, "User declined autogen");
    server_ipc_write(sock, RESPONSE_ERROR, ACCOUNT_NOT_LOADED);
    return;
  }
  secFree(prompt_text);

  char* name_suggestion = getTopHost(issuer);
  signal(SIGINT, SIG_IGN);
  askOrNeedName(account, NULL, NULL, 0, 1, name_suggestion);
  signal(SIGINT, SIG_DFL);
  secFree(name_suggestion);
  if (account_getName(account) == NULL) {  // user canceled prompt
    secFreeAccount(account);
    server_ipc_write(sock, RESPONSE_ERROR, ACCOUNT_NOT_LOADED);
    return;
  }
  char* shortname = oidc_strcopy(account_getName(account));

  char* flow = listToJSONArrayString((list_t*)flows);
  if (flow == NULL) {
    flow = oidc_strcopy(account_getMytokenUrl(account)
                            ? "[\"" FLOW_VALUE_MT_OIDC "\"]"
                            : "[\"" FLOW_VALUE_DEVICE "\",\"" FLOW_VALUE_CODE
                              "\"]");
  }
  char* account_json = accountToJSONString(account);
  secFreeAccount(account);
  agent_log(DEBUG, "sending gen request to oidcd");
  char* gen_res = ipc_communicateThroughPipe(pipes, REQUEST_GEN, account_json,
                                             flow, "{}", 0, 0, 0);
  secFree(account_json);
  secFree(flow);
  agent_log(DEBUG, "parsing response of gen");
  _parseInternalGen(pipes, sock, gen_res, original_client_req, RESPONSE_ERROR,
                    ACCOUNT_NOT_LOADED, shortname, 0);
  secFree(shortname);
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
  _parseInternalGen(pipes, sock, reauth_res, original_client_req, "%s",
                    oidcd_res, shortname, 1);
  secFree(shortname);
}

void handleOidcdComm(struct ipcPipe pipes, int sock, const char* msg,
                     const struct arguments* arguments) {
  char* send = oidc_strcopy(msg);
  INIT_KEY_VALUE(IPC_KEY_REQUEST, OIDC_KEY_REFRESHTOKEN, IPC_KEY_SHORTNAME,
                 IPC_KEY_APPLICATIONHINT, IPC_KEY_ISSUERURL, OIDC_KEY_ERROR,
                 IPC_KEY_INFO, INT_IPC_KEY_ACTION, OIDC_KEY_SCOPE);
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
                   error, info, action, scope);
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
    statlog(oidcd_res);
    secFree(oidcd_res);
    if (strequal(_request, INT_REQUEST_VALUE_UPD_REFRESH)) {
      oidc_error_t e = updateRefreshToken(_shortname, _refresh_token);
      send           = e == OIDC_SUCCESS ? oidc_strcopy(RESPONSE_SUCCESS)
                                         : oidc_sprintf(RESPONSE_ERROR, oidc_serror());
      SEC_FREE_KEY_VALUES();
      continue;
    } else if (strequal(_request, INT_REQUEST_VALUE_UPD_ISSUER)) {
      oidcp_updateIssuerConfig(_action, _issuer, _shortname);
      send = oidc_strcopy(RESPONSE_SUCCESS);
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
    } else if (strequal(_request, INT_REQUEST_VALUE_AUTOGEN)) {
      handleAutoGen(pipes, sock, msg, _issuer, _scope, _application_hint);
      SEC_FREE_KEY_VALUES();
      return;
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
    } else if (strequal(_request, INT_REQUEST_VALUE_CONFIRMMYTOKEN)) {
      char* data = askpass_getMytokenConfirmation(_info);
      send = data != NULL ? oidc_sprintf(RESPONSE_SUCCESS_INFO_OBJECT, data)
                          : oidc_sprintf(INT_RESPONSE_ERROR, oidc_errno);
      secFree(data);
      SEC_FREE_KEY_VALUES();
      continue;
    } else if (strequal(_request, INT_REQUEST_VALUE_QUERY_ACCDEFAULT)) {
      const char* account = NULL;
      if (strValid(_issuer)) {      // default for this issuer
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
