#define _XOPEN_SOURCE 700
#include "oidcs.h"

#include "account/account.h"
#include "defines/ipc_values.h"
#include "defines/settings.h"
#include "ipc/cryptCommunicator.h"
#include "ipc/cryptIpc.h"
#include "ipc/pipe.h"
#include "ipc/serveripc.h"
#include "ipc/tcp_serveripc.h"
#include "oidc-agent-server/oidc-agent-server_options.h"
#include "oidc-agent/daemonize.h"
#include "oidc-agent/oidcp/proxy_handler.h"
#include "oidc-agent/oidcp/start_oidcd.h"
#ifndef __APPLE__
#include "privileges/agent_privileges.h"
#endif
#include "utils/agentLogger.h"
#include "utils/crypt/crypt.h"
#include "utils/crypt/cryptUtils.h"
#include "utils/db/connection_db.h"
#include "utils/disableTracing.h"
#include "utils/file_io/cryptFileUtils.h"
#include "utils/file_io/file_io.h"
#include "utils/json.h"
#include "utils/memory.h"
#include "utils/oidc_error.h"
#include "utils/parseJson.h"
#include "utils/printer.h"
#include "utils/stringUtils.h"

#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>

int main(int argc, char** argv) {
  platform_disable_tracing();
  agent_openlog("oidc-agent-server.p");
  logger_setloglevel(NOTICE);
  struct oidcs_arguments arguments;

  /* Set argument defaults */
  initServerArguments(&arguments);
  srandom(time(NULL));

  argp_parse(&server_argp, argc, argv, 0, 0, &arguments);
  if (arguments.debug) {
    logger_setloglevel(DEBUG);
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
      printStdout("unset %s;\n", OIDC_PID_ENV_NAME);
      printStdout("echo Agent pid %d killed;\n", pid);
      exit(EXIT_SUCCESS);
    }
  }
  setenv(OIDC_CONFIG_DIR_ENV_NAME, arguments.data_dir,
         1);  // When a refresh token must be updated the agent will look in
              // this directory so it must be set to the data_dir

  struct connection* listencon = secAlloc(sizeof(struct connection));
  signal(SIGPIPE, SIG_IGN);
  if (ipc_tcp_server_init(listencon, arguments.port) != OIDC_SUCCESS) {
    printError("%s\n", oidc_serror());
    exit(EXIT_FAILURE);
  }
  if (!arguments.console) {
    daemonize();
  }

  struct arguments oidcd_arguments = {.no_scheme            = 1,
                                      .no_autoload          = 1,
                                      .no_webserver         = 1,
                                      .confirm              = 0,
                                      .debug                = arguments.debug,
                                      .seccomp              = 0,
                                      .always_allow_idtoken = 0,
                                      .log_console = arguments.log_console};
  struct ipcPipe   pipes           = startOidcd(&oidcd_arguments);

  if (ipc_tcp_bindAndListen(listencon) == OIDC_EBIND) {
    exit(OIDC_EBIND);
  }

  handleClientComm(listencon, pipes, arguments.data_dir);

  return EXIT_FAILURE;
}

void handleClientComm(struct connection* listencon, struct ipcPipe pipes,
                      const char* data_dir) {
  connectionDB_new();
  connectionDB_setFreeFunction((void (*)(void*)) & _secFreeConnection);
  connectionDB_setMatchFunction((matchFunction)connection_comparator);

  while (1) {
    struct connection* con =
        ipc_readAsyncFromMultipleConnectionsWithTimeout(*listencon, 0);
    if (con == NULL) {  // timeout reached
      agent_log(ERROR, "This is weird.");
      continue;
    }
    char* q = server_ipc_read(*(con->msgsock));
    if (q == NULL) {
      server_ipc_writeOidcErrnoPlain(*(con->msgsock));
    } else {  // NULL != q
      INIT_KEY_VALUE(IPC_KEY_REQUEST, IPC_KEY_CONFIG, IPC_KEY_SHORTNAME);
      if (CALL_GETJSONVALUES(q) < 0) {
        server_ipc_write(*(con->msgsock), RESPONSE_BADREQUEST, oidc_serror());
      } else {
        KEY_VALUE_VARS(request, config, id);
        if (_request) {
          if (strequal(_request, REQUEST_VALUE_CHECK)) {
            server_ipc_write(*(con->msgsock), RESPONSE_SUCCESS);
          } else if (strequal(_request, REQUEST_VALUE_ADD)) {
            handleAdd(*(con->msgsock), _config, data_dir);
          } else if (strequal(_request, REQUEST_VALUE_REMOVE)) {
            handleRemove(*(con->msgsock), _id, data_dir);
          } else if (strequal(_request, REQUEST_VALUE_ACCESSTOKEN)) {
            handleToken(pipes, *(con->msgsock), _id, q, data_dir);
          } else {
            server_ipc_write(*(con->msgsock), RESPONSE_BADREQUEST,
                             "Unsupported request type");
          }
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

void handleAdd(int sock, const char* config, const char* data_dir) {
  agent_log(DEBUG, "handle Add request");
  if (config == NULL) {
    server_ipc_write(sock, RESPONSE_BADREQUEST,
                     "Required parameter config missing");
    return;
  }
  char* shortname = NULL;
  char* file_path = NULL;
  while (1) {
    shortname = randomString(6);
    file_path = oidc_sprintf("%s/%s", data_dir, shortname);
    if (!fileDoesExist(file_path)) {
      break;
    }
    secFree(shortname);
    secFree(file_path);
  }
  agent_log(DEBUG, "Created new shortname: %s", shortname);
  struct oidc_account* account = getAccountFromJSON(config);
  if (account == NULL) {
    server_ipc_write(sock, RESPONSE_BADREQUEST,
                     "Could not parse account config");
    secFree(shortname);
    secFree(file_path);
    return;
  }
  agent_log(DEBUG, "Parsed account config into struct");
  account_setName(account, oidc_strcopy(shortname), NULL);
  agent_log(DEBUG, "Updated name");
  char* write_config = accountToJSONStringWithoutCredentials(account);
  agent_log(DEBUG, "Converted struct back to string");
  secFreeAccount(account);
  if (write_config == NULL) {
    server_ipc_write(sock, RESPONSE_ERROR, "Internal error");
    secFree(shortname);
    secFree(file_path);
    return;
  }
  char* password = randomString(14);
  char* id       = oidc_strcat(shortname, password);
  secFree(shortname);
  agent_log(DEBUG, "Created password and id");
  if (encryptAndWriteToFile(write_config, file_path, password) !=
      OIDC_SUCCESS) {
    server_ipc_writeOidcErrno(sock);
    secFree(file_path);
    secFree(password);
    secFree(id);
    secFree(write_config);
    return;
  }
  agent_log(DEBUG, "Wrote encrypted config");
  secFree(write_config);
  secFree(file_path);
  secFree(password);
  char* info = oidc_sprintf(
      "Account available in agent. Use '%s' as a shortname to obtain tokens or "
      "unload the config.\nExample:\noidc-token %s",
      id, id);
  secFree(id);
  server_ipc_write(sock, RESPONSE_SUCCESS_INFO, info);
  secFree(info);
}

#define ASSERT_ID(id)                                                       \
  do {                                                                      \
    if ((id) == NULL) {                                                     \
      server_ipc_write(sock, RESPONSE_BADREQUEST,                           \
                       "Required parameter " IPC_KEY_SHORTNAME " missing"); \
      return;                                                               \
    }                                                                       \
    if (strlen((id)) != 20) {                                               \
      oidc_errno = OIDC_ENOACCOUNT;                                         \
      server_ipc_writeOidcErrno(sock);                                      \
      return;                                                               \
    }                                                                       \
  } while (0)

#define SPLIT_ID(id)                                            \
  char* shortname = secAlloc(6 + 1);                            \
  strncpy(shortname, (id), 6);                                  \
  char* file_path = oidc_sprintf("%s/%s", data_dir, shortname); \
  char* password  = secAlloc(14 + 1);                           \
  strncpy(password, (id) + 6, 14);                              \
  agent_log(DEBUG, "Parsed id")

void handleRemove(int sock, const char* id, const char* data_dir) {
  agent_log(DEBUG, "Handle Remove Request '%s'", id);
  ASSERT_ID(id);
  SPLIT_ID(id);
  secFree(shortname);
  if (!fileDoesExist(file_path)) {
    server_ipc_write(sock, RESPONSE_SUCCESS);
    secFree(file_path);
    secFree(password);
    return;
  }
  char* config = decryptFile(file_path, password);
  secFree(password);
  agent_log(DEBUG, "Decrypted file");
  struct oidc_account* account = getAccountFromJSON(config);
  secFree(config);
  agent_log(DEBUG, "Parsed file");
  if (account == NULL) {  // Decryption failed
    oidc_errno = OIDC_ENOACCOUNT;
    server_ipc_writeOidcErrno(sock);
    secFree(file_path);
    return;
  }
  secFreeAccount(account);
  if (removeFile(file_path) != 0) {
    server_ipc_writeOidcErrno(sock);
  } else {
    agent_log(DEBUG, "Removed file");
    server_ipc_write(sock, RESPONSE_SUCCESS);
  }
  secFree(file_path);
}

char* _communicateOIDCD(struct ipcPipe pipes, int sock, const char* message,
                        const char* password, int allowInternalRequests) {
  INIT_KEY_VALUE(IPC_KEY_REQUEST, OIDC_KEY_REFRESHTOKEN, IPC_KEY_SHORTNAME);
  char* oidcd_res = ipc_communicateThroughPipe(pipes, message);
  if (oidcd_res == NULL) {
    if (oidc_errno == OIDC_EIPCDIS || oidc_errno == OIDC_EWRITE) {
      agent_log(ERROR, "oidcd died");
      server_ipc_write(sock, RESPONSE_ERROR, "oidcd died");
      exit(EXIT_FAILURE);
    }
    agent_log(ERROR, "no response from oidcd");
    return NULL;
  }
  if (!allowInternalRequests) {
    return oidcd_res;
  }
  // check response, it might be an internal request
  if (CALL_GETJSONVALUES(oidcd_res) < 0) {
    secFree(oidcd_res);
    SEC_FREE_KEY_VALUES();
    return NULL;
  }
  KEY_VALUE_VARS(request, refresh_token, shortname);
  if (_request == NULL) {  // if the response is the final response, return it
    SEC_FREE_KEY_VALUES();
    return oidcd_res;
  }
  secFree(oidcd_res);
  if (strequal(_request, INT_REQUEST_VALUE_UPD_REFRESH)) {
    oidc_error_t e =
        updateRefreshTokenUsingPassword(_shortname, _refresh_token, password);
    char* msg = e == OIDC_SUCCESS ? oidc_strcopy(RESPONSE_SUCCESS)
                                  : oidc_sprintf(RESPONSE_ERROR, oidc_serror());
    SEC_FREE_KEY_VALUES();
    char* ret =
        _communicateOIDCD(pipes, sock, msg, password, allowInternalRequests);
    secFree(msg);
    return ret;
  } else {
    oidc_setInternalError("Unknown internal request");
    SEC_FREE_KEY_VALUES();
    return NULL;
  }
}

oidc_error_t _unload(struct ipcPipe pipes, int sock, const char* shortname,
                     const char* password) {
  agent_log(DEBUG, "Unloading account '%s' from oidcd", shortname);
  char* msg       = oidc_sprintf(REQUEST_REMOVE, shortname);
  char* oidcd_res = _communicateOIDCD(pipes, sock, msg, password, 0);
  secFree(msg);
  if (oidcd_res == NULL) {
    return oidc_errno;
  }
  char* error = parseForError(oidcd_res);
  if (error != NULL) {
    agent_log(ERROR, error);
    oidc_setInternalError(error);
    return oidc_errno;
  }
  agent_log(DEBUG, "Unloaded account '%s' from oidcd", shortname);
  return OIDC_SUCCESS;
}

char* _token(struct ipcPipe pipes, int sock, const char* complete_request,
             const char* shortname, const char* password) {
  agent_log(DEBUG, "Forwarding token request for account '%s' to oidcd",
            shortname);
  // forward token request with substituted account
  cJSON* json = stringToJson(complete_request);
  if (setJSONValue(json, IPC_KEY_SHORTNAME, shortname) != OIDC_SUCCESS) {
    secFreeJson(json);
    return NULL;
  }
  char* req = jsonToString(json);
  secFreeJson(json);
  if (req == NULL) {
    return NULL;
  }
  char* oidcd_res = _communicateOIDCD(pipes, sock, req, password, 1);
  secFree(req);
  agent_log(DEBUG, "Received token response from oidcd");
  return oidcd_res;
}

oidc_error_t _load(struct ipcPipe pipes, int sock, const char* file_path,
                   const char* password) {
  agent_log(DEBUG, "Loading account '%s' into oidcd", file_path);
  if (!fileDoesExist(file_path)) {
    oidc_errno = OIDC_ENOACCOUNT;
    return oidc_errno;
  }
  char* config = decryptFile(file_path, password);
  agent_log(DEBUG, "Decrypted file");
  struct oidc_account* account = getAccountFromJSON(config);
  agent_log(DEBUG, "Parsed file");
  if (account == NULL) {  // Decryption failed
    return OIDC_ECRED;
  }
  secFreeAccount(account);
  char* req = oidc_sprintf("{\"" IPC_KEY_REQUEST "\":\"" REQUEST_VALUE_ADD
                           "\",\"" IPC_KEY_CONFIG "\":%s}",
                           config);
  secFree(config);
  char* oidcd_res = _communicateOIDCD(pipes, sock, req, password, 1);
  secFree(req);
  char* error = parseForError(oidcd_res);
  if (error != NULL) {
    agent_log(ERROR, error);
    oidc_setInternalError(error);
    return oidc_errno;
  }
  agent_log(DEBUG, "Loaded account '%s' into oidcd", file_path);
  return OIDC_SUCCESS;
}

void handleToken(struct ipcPipe pipes, int sock, const char* id,
                 const char* complete_request, const char* data_dir) {
  agent_log(DEBUG, "Handle Token Request '%s'", id);
  ASSERT_ID(id);
  SPLIT_ID(id);  // Create shortname, file_path, password

  oidc_error_t ok = _load(pipes, sock, file_path, password);
  secFree(file_path);
  switch (ok) {
    case OIDC_SUCCESS: break;
    case OIDC_ENOACCOUNT:
      server_ipc_writeOidcErrno(sock);
      secFree(shortname);
      secFree(password);
      return;
    case OIDC_ECRED:
      oidc_errno = OIDC_ENOACCOUNT;
      server_ipc_writeOidcErrno(sock);
      secFree(shortname);
      secFree(password);
      return;
    default:
      server_ipc_writeOidcErrno(sock);
      secFree(shortname);
      secFree(password);
      return;
  }

  char* token_response =
      _token(pipes, sock, complete_request, shortname, password);
  oidc_error_t oidc_errno_afterTokenResponse = oidc_errno;

  ok = _unload(pipes, sock, shortname, password);
  secFree(shortname);
  secFree(password);
  if (ok != OIDC_SUCCESS) {
    secFree(token_response);
    server_ipc_writeOidcErrno(sock);
    return;
  }

  if (token_response == NULL) {
    secFree(token_response);
    oidc_errno = oidc_errno_afterTokenResponse;
    server_ipc_writeOidcErrno(sock);
    return;
  }
  server_ipc_write(sock, token_response);
  secFree(token_response);
}
