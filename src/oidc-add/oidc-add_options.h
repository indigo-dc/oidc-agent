#ifndef OIDC_ADD_OPTIONS_H
#define OIDC_ADD_OPTIONS_H

#include "utils/lifetimeArg.h"
#include "utils/stringUtils.h"

#include <argp.h>

struct arguments {
  char* args[1]; /* account */
  char* pw_cmd;

  unsigned char remove;
  unsigned char removeAll;
  unsigned char debug;
  unsigned char verbose;
  unsigned char listConfigured;
  unsigned char listLoaded;
  unsigned char print;
  unsigned char lock;
  unsigned char unlock;
  unsigned char seccomp;
  unsigned char pw_keyring;
  unsigned char confirm;

  struct lifetimeArg pw_lifetime;
  struct lifetimeArg lifetime;
};

#define OPT_SECCOMP 1
#define OPT_PW_STORE 2
#define OPT_PW_KEYRING 3
#define OPT_PW_CMD 4

static struct argp_option options[] = {
    {0, 0, 0, 0, "General:", 1},
    {"remove", 'r', 0, 0, "The account configuration is removed, not added", 1},
    {"remove-all", 'R', 0, 0,
     "Removes all account configurations currently loaded", 1},
    {"list", 'l', 0, 0, "Lists all configured account configurations", 1},
    {"loaded", 'a', 0, 0, "Lists the currently loaded account configurations",
     1},
    {"print", 'p', 0, 0, "Prints the encrypted account configuration and exits",
     1},
    {"lifetime", 't', "TIME", 0,
     "Set a maximum lifetime in seconds when adding the account configuration",
     1},
    {"lock", 'x', 0, 0, "Lock agent", 1},
    {"unlock", 'X', 0, 0, "Unlock agent", 1},
    {"pw-store", OPT_PW_STORE, "TIME", 1,
     "Keeps the encryption password encrypted in memory for TIME seconds. "
     "Default value for TIME: Forever",
     1},
#ifndef __APPLE__
    {"pw-keyring", OPT_PW_KEYRING, 0, 0,
     "Stores the used encryption password in the systems' keyring", 1},
#endif
    {"pw-cmd", OPT_PW_CMD, "CMD", 0,
     "Command from which the agent can read the encryption password", 1},
    {"confirm", 'c', 0, 0,
     "Require user confirmation when an application requests an access token "
     "for this configuration",
     1},
#ifndef __APPLE__
    {"seccomp", OPT_SECCOMP, 0, 0,
     "Enables seccomp system call filtering; allowing only predefined system "
     "calls.",
     1},
#endif

    {0, 0, 0, 0, "Verbosity:", 2},
    {"debug", 'g', 0, 0, "Sets the log level to DEBUG", 2},
    {"verbose", 'v', 0, 0, "Enables verbose mode", 2},

    {0, 0, 0, 0, "Help:", -1},
    {0, 'h', 0, OPTION_HIDDEN, 0, -1},
    {0, 0, 0, 0, 0, 0}};

#define ARG_PROVIDED_BUT_USES_DEFAULT 2

static error_t parse_opt(int key, char* arg, struct argp_state* state) {
  struct arguments* arguments = state->input;
  switch (key) {
    case 'r': arguments->remove = 1; break;
    case 'R': arguments->removeAll = 1; break;
    case 'g': arguments->debug = 1; break;
    case 'v': arguments->verbose = 1; break;
    case 'p': arguments->print = 1; break;
    case 'l': arguments->listConfigured = 1; break;
    case 'a': arguments->listLoaded = 1; break;
    case 'x': arguments->lock = 1; break;
    case 'X': arguments->unlock = 1; break;
    case 'c': arguments->confirm = 1; break;
    case OPT_PW_CMD: arguments->pw_cmd = arg; break;
    case OPT_PW_KEYRING: arguments->pw_keyring = 1; break;
    case OPT_PW_STORE:
      if (arg == NULL) {
        arguments->pw_lifetime.argProvided = ARG_PROVIDED_BUT_USES_DEFAULT;
        break;
      }
      if (!isdigit(*arg)) {
        return ARGP_ERR_UNKNOWN;
      }
      arguments->pw_lifetime.lifetime    = strToULong(arg);
      arguments->pw_lifetime.argProvided = 1;
      break;
    case 't':
      if (!isdigit(*arg)) {
        return ARGP_ERR_UNKNOWN;
      }
      arguments->lifetime.lifetime    = strToInt(arg);
      arguments->lifetime.argProvided = 1;
      break;
    case OPT_SECCOMP: arguments->seccomp = 1; break;
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
      if (arguments->listConfigured || arguments->listLoaded ||
          arguments->lock || arguments->unlock || arguments->removeAll) {
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

static char args_doc[] = "ACCOUNT_SHORTNAME | -a | -l | -x | -X | -R";

static char doc[] =
    "oidc-add -- A client for adding and removing accounts to the oidc-agent";

static struct argp argp = {options, parse_opt, args_doc, doc, 0, 0, 0};

static inline void initArguments(struct arguments* arguments) {
  arguments->remove                  = 0;
  arguments->removeAll               = 0;
  arguments->debug                   = 0;
  arguments->verbose                 = 0;
  arguments->listConfigured          = 0;
  arguments->listLoaded              = 0;
  arguments->print                   = 0;
  arguments->lifetime.argProvided    = 0;
  arguments->lifetime.lifetime       = 0;
  arguments->lock                    = 0;
  arguments->unlock                  = 0;
  arguments->args[0]                 = NULL;
  arguments->seccomp                 = 0;
  arguments->pw_lifetime.argProvided = 0;
  arguments->pw_lifetime.lifetime    = 0;
  arguments->pw_keyring              = 0;
  arguments->pw_cmd                  = NULL;
  arguments->confirm                 = 0;
}

#endif  // OIDC_ADD_OPTIONS_H
