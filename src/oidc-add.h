#ifndef OIDC_ADD_H
#define OIDC_ADD_H

#include "add_handler.h"
#include "oidc_error.h"
#include "version.h"

#include <argp.h>
#include <stdlib.h>
#include <time.h>

#define OIDC_SOCK_ENV_NAME "OIDC_SOCK"

const char* argp_program_version = ADD_VERSION;

const char* argp_program_bug_address = BUG_ADDRESS;

struct arguments {
  char*              args[1]; /* account */
  int                remove;
  int                debug;
  int                verbose;
  int                list;
  int                print;
  struct lifetimeArg lifetime;
  int                lock;
  int                unlock;
};

static struct argp_option options[] = {
    {0, 0, 0, 0, "General:", 1},
    {"remove", 'r', 0, 0, "The account configuration is removed, not added", 1},
    {"list", 'l', 0, 0, "Lists the available account configurations", 1},
    {"print", 'p', 0, 0, "Prints the encrypted account configuration and exits",
     1},
    {"lifetime", 't', "LIFETIME", 0,
     "Set a maximum lifetime in seconds when adding the account configuration",
     1},
    {"lock", 'x', 0, 0, "Lock agent", 1},
    {"unlock", 'X', 0, 0, "Unlock agent", 1},
    {0, 0, 0, 0, "Verbosity:", 2},
    {"debug", 'g', 0, 0, "Sets the log level to DEBUG", 2},
    {"verbose", 'v', 0, 0, "Enables verbose mode", 2},
    {0, 0, 0, 0, "Help:", -1},
    {0, 'h', 0, OPTION_HIDDEN, 0, -1},
    {0, 0, 0, 0, 0, 0}};

static error_t parse_opt(int key, char* arg, struct argp_state* state) {
  struct arguments* arguments = state->input;
  switch (key) {
    case 'r': arguments->remove = 1; break;
    case 'g': arguments->debug = 1; break;
    case 'v': arguments->verbose = 1; break;
    case 'p': arguments->print = 1; break;
    case 'l': arguments->list = 1; break;
    case 'x': arguments->lock = 1; break;
    case 'X': arguments->unlock = 1; break;
    case 't':
      if (!isdigit(*arg)) {
        return ARGP_ERR_UNKNOWN;
      }
      arguments->lifetime.lifetime    = atoi(arg);
      arguments->lifetime.argProvided = 1;
      break;
    case 'h':
      argp_state_help(state, state->out_stream, ARGP_HELP_STD_HELP);
      break;
    case ARGP_KEY_ARG:
      if (state->arg_num >= 1) {
        argp_usage(state);
      }
      arguments->args[state->arg_num] = arg;
      break;
    case ARGP_KEY_END:
      if (arguments->list || arguments->lock || arguments->unlock) {
        break;
      }
      if (state->arg_num < 1) {
        argp_usage(state);
      }
      break;
    default: return ARGP_ERR_UNKNOWN;
  }
  return 0;
}

static char args_doc[] = "ACCOUNT_SHORTNAME | -l | -x | -X";

static char doc[] =
    "oidc-add -- A client for adding and removing accounts to the oidc-agent";

static struct argp argp = {options, parse_opt, args_doc, doc, 0, 0, 0};

void initArguments(struct arguments* arguments) {
  arguments->remove               = 0;
  arguments->debug                = 0;
  arguments->verbose              = 0;
  arguments->list                 = 0;
  arguments->print                = 0;
  arguments->lifetime.argProvided = 0;
  arguments->lifetime.lifetime    = 0;
  arguments->lock                 = 0;
  arguments->unlock               = 0;
  arguments->args[0]              = NULL;
}

#endif  // OIDC_ADD_H
