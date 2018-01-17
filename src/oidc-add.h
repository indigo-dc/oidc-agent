#ifndef OIDC_ADD_H
#define OIDC_ADD_H

#include "version.h"
#include "oidc_error.h"

#include <argp.h>

#define OIDC_SOCK_ENV_NAME "OIDC_SOCK"

const char *argp_program_version = ADD_VERSION;

const char *argp_program_bug_address = BUG_ADDRESS;

struct arguments {
  char* args[1];            /* account */
  int remove;
  int debug;
  int verbose;
};

static struct argp_option options[] = {
  {"remove", 'r', 0, 0, "the account config is removed, not added", 0},
  {"debug", 'g', 0, 0, "sets the log level to DEBUG", 0},
  {"verbose", 'v', 0, 0, "enables verbose mode. The send data will be printed.", 0},
  {0, 0, 0, 0, 0, 0}
};

static error_t parse_opt (int key, char *arg, struct argp_state *state) {
  struct arguments *arguments = state->input;
  switch (key) {
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

static char args_doc[] = "ACCOUNT_SHORTNAME";

static char doc[] = "oidc-add -- A client for adding and removing accounts to the oidc-agent";

static struct argp argp = {options, parse_opt, args_doc, doc, 0, 0, 0};

void initArguments(struct arguments* arguments) {
  arguments->remove = 0;
  arguments->debug = 0;
  arguments->verbose = 0;
  arguments->args[0]=NULL;
}

#endif //OIDC_ADD_H
