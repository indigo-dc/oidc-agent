#ifndef OIDC_TOKEN_H
#define OIDC_TOKEN_H

#include "version.h"

#include <argp.h>
#include <stdlib.h>

const char *argp_program_version = TOKEN_VERSION;

const char *argp_program_bug_address = BUG_ADDRESS;

struct arguments {
  char* args[1];            /* account shortname */
  int list_accounts;
  unsigned long min_valid_period;  
};

static struct argp_option options[] = {
  {"listaccounts", 'l', 0, 0, "Lists the currently loaded accounts", 0},
  {"time",  't', "min_valid_period", 0, "period of how long the access token should be at least valid in seconds", 0},
  {0, 0, 0, 0, 0, 0}
};

static error_t parse_opt (int key, char *arg, struct argp_state *state) {
  struct arguments *arguments = state->input;

  switch (key)
  {
    case 'l':
      arguments->list_accounts = 1;
      break;
    case 't':
      if(!isdigit(*arg)) {
        return ARGP_ERR_UNKNOWN;
      }
      arguments->min_valid_period = atoi(arg);
      break;
    case ARGP_KEY_ARG:
      if(state->arg_num >= 1) {
        argp_usage(state);
      }
      arguments->args[state->arg_num] = arg;
      break;
    case ARGP_KEY_END:
      if(arguments->list_accounts) {
        break;
      }
      if (state->arg_num < 1) {
        argp_usage (state);
      }
      break;
    default:
      return ARGP_ERR_UNKNOWN;
  }
  return 0;
}

static char args_doc[] = "ACCOUNT_SHORTNAME | -l";

static char doc[] = "oidc-token -- A client for oidc-agent for getting OIDC access tokens.";

static struct argp argp = {options, parse_opt, args_doc, doc, 0, 0, 0};

static void initArguments(struct arguments* arguments) {
  arguments->min_valid_period = 0;
  arguments->list_accounts = 0;
  arguments->args[0]=NULL;
}



#endif // OIDC_TOKEN_H
