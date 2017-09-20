#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <argp.h>
#include <ctype.h>
#include <syslog.h>

#include "account.h"
#include "prompt.h"
#include "ipc.h"
#include "oidc_utilities.h"
#include "version.h"
#include "file_io.h"

#define OIDC_SOCK_ENV_NAME "OIDC_SOCK"

const char *argp_program_version = ADD_VERSION;

const char *argp_program_bug_address = BUG_ADDRESS;

/* This structure is used by main to communicate with parse_opt. */
struct arguments {
  char* args[1];            /* account */
  int remove;
  int debug;
  int verbose;
};

/*
   OPTIONS.  Field 1 in ARGP.
   Order of fields: {NAME, KEY, ARG, FLAGS, DOC}.
   */
static struct argp_option options[] = {
  {"remove", 'r', 0, 0, "the account config is removed, not added", 0},
  {"debug", 'g', 0, 0, "sets the log level to DEBUG", 0},
  {"verbose", 'v', 0, 0, "enables verbose mode. The send data will be printed.", 0},
  {0}
};

/*
   PARSER. Field 2 in ARGP.
   Order of parameters: KEY, ARG, STATE.
   */
static error_t parse_opt (int key, char *arg, struct argp_state *state) {
  struct arguments *arguments = state->input;

  switch (key)
  {
    case 'r':
      arguments->remove = 1;
      break;
    case 'g':
      arguments->debug = 1;
      break;
    case 'v':
      arguments->verbose = 1;
      break;
    case ARGP_KEY_ARG:
      if(state->arg_num >= 1) {
        argp_usage(state);
      }
      arguments->args[state->arg_num] = arg;
      break;
    case ARGP_KEY_END:
      if(state->arg_num < 1) {
        argp_usage (state);
      }
      break;
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
static char args_doc[] = "ACCOUNT_SHORTNAME";

/*
   DOC.  Field 4 in ARGP.
   Program documentation.
   */
static char doc[] = "oidc-add -- A client for adding and removing accounts to the oidc-agent";

/*
   The ARGP structure itself.
   */
static struct argp argp = {options, parse_opt, args_doc, doc};

//TODO check refactor
int main(int argc, char** argv) {
  openlog("oidc-add", LOG_CONS|LOG_PID, LOG_AUTHPRIV);
  setlogmask(LOG_UPTO(LOG_NOTICE));
  struct arguments arguments;

  /* Set argument defaults */
  arguments.remove = 0;
  arguments.debug = 0;
  arguments.verbose = 0;
  arguments.args[0]=NULL;

  argp_parse (&argp, argc, argv, 0, 0, &arguments);
  if(arguments.debug) {
    setlogmask(LOG_UPTO(LOG_DEBUG));
  }
  if(getOidcDir()==NULL) {
    printf("Error: oidc-dir does not exist. Run make to create it.\n");
    exit(EXIT_FAILURE);
  }

  char* account = arguments.args[0];

  if(!accountConfigExists(account)) {
    printf("No account configured with that short name\n");
    exit(EXIT_FAILURE);
  }
  struct oidc_account* p = NULL;
  while(NULL==p) {
    char* password = promptPassword("Enter encryption password for account config %s: ", account);
    p = decryptAccount(account, password);
    clearFreeString(password);
  }
  char* json_p = accountToJSON(*p);
  freeAccount(p);

  struct connection con = {0,0,0};
  if(ipc_init(&con, OIDC_SOCK_ENV_NAME, 0)!=0) {
    exit(EXIT_FAILURE);
  }
  if(ipc_connect(con)<0) {
    printf("Could not connect to oidc-agent\n");
    exit(EXIT_FAILURE);
  }
  ipc_write(*(con.sock), REQUEST_CONFIG, arguments.remove ? "remove" : "add", json_p);
  if(arguments.verbose) {
    printf("The following data was sent to oidc-agent:\n%s\n", json_p);
  }
  clearFreeString(json_p);
  char* res = ipc_read(*(con.sock));
  ipc_close(&con);
  if(NULL==res) {
    printf("An unexpected error occured. It's seems that oidc-agent has stopped.\n That's not good.");
    exit(EXIT_FAILURE);
  }

  struct key_value pairs[2];
  pairs[0].key = "status";
  pairs[1].key = "error";
  if(getJSONValues(res, pairs, sizeof(pairs)/sizeof(*pairs))<0) {
    printf("Could not decode json: %s\n", res);
    printf("This seems to be a bug. Please hand in a bug report.\n");
    clearFreeString(res);
    exit(EXIT_FAILURE);
  }
  clearFreeString(res);
  if(pairs[1].value!=NULL) {
    printf("Error: %s\n", pairs[1].value);
    clearFreeString(pairs[1].value); clearFreeString(pairs[0].value);
    exit(EXIT_FAILURE);
  }
  printf("%s\n", pairs[0].value);
  clearFreeString(pairs[0].value);
  return EXIT_SUCCESS;
}
