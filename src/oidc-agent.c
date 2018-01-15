#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <syslog.h>
#include <signal.h>
#include <time.h>
#include <argp.h>
#include <ctype.h>
#include <libgen.h>

#include "oidc-agent.h"
#include "oidc.h"
#include "ipc.h"
#include "account.h"
#include "oidc_utilities.h"
#include "oidc_error.h"
#include "version.h"
#include "settings.h"

const char *argp_program_version = AGENT_VERSION;

const char *argp_program_bug_address = BUG_ADDRESS;

/* This structure is used by main to communicate with parse_opt. */
struct arguments {
  int kill_flag;
  int debug;
  int console;
};

/*
   OPTIONS.  Field 1 in ARGP.
   Order of fields: {NAME, KEY, ARG, FLAGS, DOC}.
   */
static struct argp_option options[] = {
  {"kill", 'k', 0, 0, "Kill the current agent (given by the OIDCD_PID environment variable).", 0},
  {"debug", 'g', 0, 0, "sets the log level to DEBUG", 0},
  {"console", 'c', 0, 0, "runs oidc-agent on the console, without daemonizing", 0},
  {0, 0, 0, 0, 0, 0}
};

/*
   PARSER. Field 2 in ARGP.
   Order of parameters: KEY, ARG, STATE.
   */
static error_t parse_opt (int key, char *arg __attribute__((unused)), struct argp_state *state) {
  struct arguments *arguments = state->input;

  switch (key) {
    case 'k':
      arguments->kill_flag = 1;
      break;
    case 'g':
      arguments->debug = 1;
      break;
    case 'c':
      arguments->console = 1;
      break;
    case ARGP_KEY_ARG:
      argp_usage(state);
    default:
      return ARGP_ERR_UNKNOWN;
  }
  return 0;
}

/*
   ARGS_DOC. Field 3 in ARGP.
   A description of the non-option command-line arguments
   that we accept.
   */
static char args_doc[] = "";

/*
   DOC.  Field 4 in ARGP.
   Program documentation.
   */
static char doc[] = "oidc-agent -- A agent to manage oidc token";

/*
   The ARGP structure itself.
   */
static struct argp argp = {options, parse_opt, args_doc, doc};


void sig_handler(int signo) {
  switch(signo) {
    case SIGSEGV:
      syslog(LOG_AUTHPRIV|LOG_EMERG, "Caught Signal SIGSEGV");
      break;
    default: 
      syslog(LOG_AUTHPRIV|LOG_EMERG, "Caught Signal %d", signo);
  }
  exit(signo);
}

void daemonize() {
  pid_t pid;
  if((pid = fork()) == -1) {
    syslog(LOG_AUTHPRIV|LOG_ALERT, "fork %m");
    exit(EXIT_FAILURE);
  } else if(pid > 0) {
    exit(EXIT_SUCCESS);
  }
  if(setsid() < 0) {
    exit(EXIT_FAILURE);
  }
  signal(SIGHUP, SIG_IGN);
  if((pid = fork()) == -1) {
    syslog(LOG_AUTHPRIV|LOG_ALERT, "fork %m");
    exit(EXIT_FAILURE);
  } else if(pid > 0) {
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

void handleGen(int sock, struct oidc_account** loaded_p, size_t* loaded_p_count, char* account_json, const char* flow) {
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "Handle Gen request");
  struct oidc_account* account = getAccountFromJSON(account_json);
  if(account==NULL) {
    ipc_writeOidcErrno(sock);
    return;
  }
  getIssuerConfig(account);
  if(!isValid(account_getTokenEndpoint(*account))) {
    ipc_writeOidcErrno(sock);
    freeAccount(account);
    return;
  }

  if(flow && strcasecmp("code", flow)==0){
    char* uri = buildCodeFlowUri(account);
    if(uri == NULL) {
      ipc_writeOidcErrno(sock);
    } else {
      ipc_write(sock, RESPONSE_STATUS_CODEURI, "accepted", uri);
    }
    freeAccount(account);
    clearFreeString(uri);
    return;
  }

  if(retrieveAccessToken(account, FORCE_NEW_TOKEN)!=OIDC_SUCCESS) {
    ipc_writeOidcErrno(sock);
    freeAccount(account);
    return;
  } 

  account_setUsername(account, NULL);
  account_setPassword(account, NULL);
  if(isValid(account_getRefreshToken(*account))) {
    char* json = accountToJSON(*account);
    ipc_write(sock, RESPONSE_STATUS_CONFIG, "success", json);
    clearFreeString(json);
    *loaded_p = removeAccount(*loaded_p, loaded_p_count, *account);
    *loaded_p = addAccount(*loaded_p, loaded_p_count, *account);
    clearFree(account, sizeof(*account));
  } else {
    if(flow==NULL && hasRedirectUris(*account)) { // TODO check if account config has redirect uris
      //TODO flow is just checked for code, check also for other values and if
      //specified only do that flow -> refactor
      char* uri = buildCodeFlowUri(account);
      if(uri == NULL) {
        ipc_writeOidcErrno(sock);
      } else {
        ipc_write(sock, RESPONSE_STATUS_CODEURI_INFO, "accepted", uri, "No flow was specified. Could not get a refresh token using refresh flow and password flow. May use the authorization code flow uri.");
      }
      freeAccount(account);
      clearFreeString(uri);
      return;

    } else {
      ipc_write(sock, RESPONSE_ERROR, "Could not get a refresh token");   
      freeAccount(account);
    }
  }
} 

void handleAdd(int sock, struct oidc_account** loaded_p, size_t* loaded_p_count, char* account_json) {
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "Handle Add request");
  struct oidc_account* account = getAccountFromJSON(account_json);
  if(account==NULL) {
    ipc_writeOidcErrno(sock);
    return;
  }
  if(NULL!=findAccount(*loaded_p, *loaded_p_count, *account)) {
    freeAccount(account);
    ipc_write(sock, RESPONSE_ERROR, "account already loaded");
    return;
  }
  getIssuerConfig(account);
  if(!isValid(account_getTokenEndpoint(*account))) {
    ipc_writeOidcErrno(sock);
    return;
  }
  if(retrieveAccessTokenRefreshFlowOnly(account, FORCE_NEW_TOKEN)!=OIDC_SUCCESS) {
    freeAccount(account);
    ipc_writeOidcErrno(sock);
    return;
  }
  *loaded_p = addAccount(*loaded_p, loaded_p_count, *account);
  clearFree(account, sizeof(*account));
  ipc_write(sock, RESPONSE_STATUS_SUCCESS);
}

void handleRm(int sock, struct oidc_account** loaded_p, size_t* loaded_p_count, char* account_json, int revoke) {
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "Handle Remove request");
  struct oidc_account* account = getAccountFromJSON(account_json);
  if(account==NULL) {
    ipc_writeOidcErrno(sock);
    return;
  }
  if(NULL==findAccount(*loaded_p, *loaded_p_count, *account)) {
    freeAccount(account);
    ipc_write(sock, RESPONSE_ERROR, revoke ? "Could not revoke token: account not loaded" : "account not loaded");
    return;
  }
  if(revoke && revokeToken(account)!=OIDC_SUCCESS) {
    freeAccount(account);
    ipc_write(sock, RESPONSE_ERROR, "Could not revoke token: %s", oidc_serror());
    return;
  }
  *loaded_p = removeAccount(*loaded_p, loaded_p_count, *account);
  freeAccount(account);
  ipc_write(sock, RESPONSE_STATUS_SUCCESS);
}

void handleToken(int sock, struct oidc_account* loaded_p, size_t loaded_p_count, char* short_name, char* min_valid_period_str) {
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "Handle Token request");
  if(short_name==NULL || min_valid_period_str== NULL) {
    ipc_write(sock, RESPONSE_ERROR, "Bad request. Need account name and min_valid_period for getting access token.");
    return;
  }
  struct oidc_account key = {0, short_name, 0};
  time_t min_valid_period = atoi(min_valid_period_str);
  struct oidc_account* account = findAccount(loaded_p, loaded_p_count, key);
  if(account==NULL) {
    ipc_write(sock, RESPONSE_ERROR, "Account not loaded.");
    return;
  }
  if(retrieveAccessTokenRefreshFlowOnly(account, min_valid_period)!=0) {
    ipc_writeOidcErrno(sock);
    return;
  }
  ipc_write(sock, RESPONSE_STATUS_ACCESS, "success", account_getAccessToken(*account));
}

void handleList(int sock, struct oidc_account* loaded_p, size_t loaded_p_count) {
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "Handle list request");
  char* accountList = getAccountNameList(loaded_p, loaded_p_count);
  ipc_write(sock, RESPONSE_STATUS_ACCOUNT, "success", oidc_errno==OIDC_EARGNULL ? "[]" : accountList);
  clearFreeString(accountList);
}

void handleRegister(int sock, struct oidc_account* loaded_p, size_t loaded_p_count, char* account_json) {
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "Handle Register request");
  struct oidc_account* account = getAccountFromJSON(account_json);
  if(account==NULL) {
    ipc_writeOidcErrno(sock);
    return;
  }
  if(NULL!=findAccount(loaded_p, loaded_p_count, *account)) {
    freeAccount(account);
    ipc_write(sock, RESPONSE_ERROR, "A account with this shortname is already loaded. I will not register a new one.");
    return;
  }
  if(getIssuerConfig(account)!=OIDC_SUCCESS) {
    freeAccount(account);
    ipc_writeOidcErrno(sock);
    return;
  }
  char* res = dynamicRegistration(account, 1);
  if(res==NULL) {
    ipc_writeOidcErrno(sock);
  } else {
    if(json_hasKey(res, "error")) { // first failed
      char* res2 = dynamicRegistration(account, 0);
      if(res2==NULL) { //second failed complety
        ipc_writeOidcErrno(sock);
      } else {
        if(json_hasKey(res2, "error")) { // first and second failed
          ipc_write(sock, RESPONSE_ERROR, res); //TODO sent both responses
        } else { // first failed, seconds successfull, still need the grant_types.
          char* error = getJSONValue(res, "error_description");
          if(error==NULL) {
            error = getJSONValue(res, "error");
          }
          char* fmt = "The client was registered with the resulting config. It is not usable for oidc-agent in that way. Please contact the provider to update the client configuration.\nprovider: %s\nclient_id: %s\nadditional needed grant_types: password";
          char* client_id = getJSONValue(res2, "client_id");
          char* send = oidc_sprintf(fmt, account_getIssuerUrl(*account), client_id);
          clearFreeString(client_id);
          if(send!=NULL) {
            ipc_write(sock, RESPONSE_ERROR_CLIENT_INFO, error, res2, send);
          } else {
            ipc_writeOidcErrno(sock);
          }
          clearFreeString(send);
          clearFreeString(error);
        }
      }
      clearFreeString(res2);
    } else { // first was successfull
      ipc_write(sock, RESPONSE_SUCCESS_CLIENT, res);
    }
  }
  clearFreeString(res);
  freeAccount(account);
}

void handleCodeExchange(int sock, struct oidc_account** loaded_p, size_t* loaded_p_count, char* account_json, char* code, char* redirect_uri) {
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "Handle codeExchange request");
  struct oidc_account* account = getAccountFromJSON(account_json);
  if(account==NULL) {
    ipc_writeOidcErrno(sock);
    return;
  }
  if(getIssuerConfig(account)!=OIDC_SUCCESS) {
    freeAccount(account);
    ipc_writeOidcErrno(sock);
    return;
  }
  if(codeExchange(account, code, redirect_uri)!=OIDC_SUCCESS) {
    freeAccount(account);
    ipc_writeOidcErrno(sock);
    return;
  }
  //TODO stop webserver
  if(isValid(account_getRefreshToken(*account))) {
    char* json = accountToJSON(*account);
    ipc_write(sock, RESPONSE_STATUS_CONFIG, "success", json);
    clearFreeString(json);
    *loaded_p = removeAccount(*loaded_p, loaded_p_count, *account);
    *loaded_p = addAccount(*loaded_p, loaded_p_count, *account);
    clearFree(account, sizeof(*account));
  } else {
    ipc_write(sock, RESPONSE_ERROR, "Could not get a refresh token");   
    freeAccount(account);
  }

}

int main(int argc, char** argv) {
  openlog("oidc-agent", LOG_CONS|LOG_PID, LOG_AUTHPRIV);
  setlogmask(LOG_UPTO(LOG_NOTICE));
  struct arguments arguments;

  /* Set argument defaults */
  arguments.kill_flag = 0;
  arguments.console = 0;
  arguments.debug = 0;

  argp_parse (&argp, argc, argv, 0, 0, &arguments);
  if(arguments.debug) {
    setlogmask(LOG_UPTO(LOG_DEBUG));
  }

  if(arguments.kill_flag) {
    char* pidstr = getenv(OIDC_PID_ENV_NAME);
    if(pidstr == NULL) {
      fprintf(stderr, "%s not set, cannot kill Agent\n", OIDC_PID_ENV_NAME);
      exit(EXIT_FAILURE);
    }
    pid_t pid = atoi(pidstr);
    if(0 == pid) {
      fprintf(stderr, "%s not set to a valid pid: %s\n", OIDC_PID_ENV_NAME, pidstr);
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

  struct connection* listencon = calloc(sizeof(struct connection), 1);
  if(ipc_init(listencon, OIDC_SOCK_ENV_NAME, 1)!=OIDC_SUCCESS) {
    fprintf(stderr, "%s\n", oidc_serror());
    exit(EXIT_FAILURE);
  }
  if(!arguments.console) {
    daemonize();
  }

  ipc_bindAndListen(listencon);

  struct oidc_account* loaded_p = NULL;
  struct oidc_account** loaded_p_addr = &loaded_p;
  size_t loaded_p_count = 0;

  struct connection* clientcons = NULL;
  struct connection** clientcons_addr = &clientcons;
  size_t number_clients = 0;

  while(1) {
    struct connection* con = ipc_async(*listencon, clientcons_addr, &number_clients);
    if(con==NULL) {
      // should never happen
      syslog(LOG_AUTHPRIV|LOG_ALERT, "Something went wrong");
    } else {
      char* q = ipc_read(*(con->msgsock));
      if(NULL!=q) {
        struct key_value pairs[7];
        pairs[0].key = "request"; pairs[0].value = NULL;
        pairs[1].key = "account"; pairs[1].value = NULL;
        pairs[2].key = "min_valid_period"; pairs[2].value = NULL;
        pairs[3].key = "config"; pairs[3].value = NULL;
        pairs[4].key = "flow"; pairs[4].value = NULL;
        pairs[5].key = "code"; pairs[5].value = NULL;
        pairs[6].key = "redirect_uri"; pairs[6].value = NULL;
        if(getJSONValues(q, pairs, sizeof(pairs)/sizeof(*pairs))<0) {
          ipc_write(*(con->msgsock), "Bad request: %s", oidc_serror());
        } else {
          if(pairs[0].value) {
            if(strcmp(pairs[0].value, "gen")==0) {
              handleGen(*(con->msgsock), loaded_p_addr, &loaded_p_count, pairs[3].value, pairs[4].value);
            } else if(strcmp(pairs[0].value, "code_exchange")==0 ) {
              handleCodeExchange(*(con->msgsock), loaded_p_addr, &loaded_p_count, pairs[3].value, pairs[5].value, pairs[6].value);
            } else if(strcmp(pairs[0].value, "add")==0) {
              handleAdd(*(con->msgsock), loaded_p_addr, &loaded_p_count, pairs[3].value);
            } else if(strcmp(pairs[0].value, "remove")==0) {
              handleRm(*(con->msgsock), loaded_p_addr, &loaded_p_count, pairs[3].value, 0);
            } else if(strcmp(pairs[0].value, "delete")==0) {
              handleRm(*(con->msgsock), loaded_p_addr, &loaded_p_count, pairs[3].value, 1);
            } else if(strcmp(pairs[0].value, "access_token")==0) {
              handleToken(*(con->msgsock), *loaded_p_addr, loaded_p_count, pairs[1].value, pairs[2].value);
            } else if(strcmp(pairs[0].value, "account_list")==0) {
              handleList(*(con->msgsock), *loaded_p_addr, loaded_p_count);
            } else if(strcmp(pairs[0].value, "register")==0) {
              handleRegister(*(con->msgsock), *loaded_p_addr, loaded_p_count, pairs[3].value);
            } else {
              ipc_write(*(con->msgsock), "Bad request. Unknown request type.");
            }
          } else {
            ipc_write(*(con->msgsock), "Bad request. No request type.");
          }
        }
        clearFreeKeyValuePairs(pairs, sizeof(pairs)/sizeof(*pairs));
        clearFreeString(q);
      }
      syslog(LOG_AUTHPRIV|LOG_DEBUG, "Remove con from pool");
      clientcons = removeConnection(*clientcons_addr, &number_clients, con);
      clientcons_addr = &clientcons;
    }
  }
  return EXIT_FAILURE;
}




