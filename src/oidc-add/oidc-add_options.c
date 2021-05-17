#include "oidc-add_options.h"

#include "defines/settings.h"
#include "utils/commonFeatures.h"
#include "utils/prompt_mode.h"
#include "utils/stringUtils.h"

#define OPT_SECCOMP 1
#define OPT_PW_STORE 2
#define OPT_PW_KEYRING 3
#define OPT_PW_CMD 4
#define OPT_ALWAYS_ALLOW_IDTOKEN 5
#define OPT_PW_PROMPT 6
#define OPT_PW_FILE 7
#define OPT_REMOTE 8
#define OPT_PW_ENV 9

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
    {"pw-env", OPT_PW_ENV, OIDC_PASSWORD_ENV_NAME, OPTION_ARG_OPTIONAL,
     "Reads the encryption password from the passed environment variable "
     "(default: " OIDC_PASSWORD_ENV_NAME "), instead of prompting the user",
     1},
    {"pw-cmd", OPT_PW_CMD, "CMD", 0,
     "Command from which the agent can read the encryption password", 1},
    {"pw-file", OPT_PW_FILE, "FILE", 0,
     "Uses the first line of FILE as the encryption password.", 1},
    {"confirm", 'c', 0, 0,
     "Require user confirmation when an application requests an access token "
     "for this configuration",
     1},
    {"pw-prompt", OPT_PW_PROMPT, "cli|gui", 0,
     "Change the mode how oidc-add should prompt for passwords. The default is "
     "'cli'.",
     1},
#ifndef __APPLE__
    {"seccomp", OPT_SECCOMP, 0, 0,
     "Enables seccomp system call filtering; allowing only predefined system "
     "calls.",
     1},
#endif
    {"always-allow-idtoken", OPT_ALWAYS_ALLOW_IDTOKEN, 0, 0,
     "Always allow id-token requests without manual approval by the user for "
     "this account configuration.",
     1},
    {"remote", OPT_REMOTE, 0, 0,
     "Use a remote central oidc-agent, instead of a local one.", 1},

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
    case 'R': arguments->removeAll = 1; break;
    case 'g': arguments->debug = 1; break;
    case 'v': arguments->verbose = 1; break;
    case 'p': arguments->print = 1; break;
    case 'l': arguments->listConfigured = 1; break;
    case 'a': arguments->listLoaded = 1; break;
    case 'x': arguments->lock = 1; break;
    case 'X': arguments->unlock = 1; break;
    case 'c': arguments->confirm = 1; break;
    case OPT_PW_ENV: arguments->pw_env = arg ?: OIDC_PASSWORD_ENV_NAME; break;
    case OPT_PW_CMD: arguments->pw_cmd = arg; break;
    case OPT_PW_FILE: arguments->pw_file = arg; break;
    case OPT_PW_KEYRING: arguments->pw_keyring = 1; break;
    case OPT_PW_PROMPT:
      if (strequal(arg, "cli")) {
        arguments->pw_prompt_mode = PROMPT_MODE_CLI;
      } else if (strequal(arg, "gui")) {
        arguments->pw_prompt_mode = PROMPT_MODE_GUI;
        common_assertOidcPrompt();
      } else {
        return ARGP_ERR_UNKNOWN;
      }
      set_pw_prompt_mode(arguments->pw_prompt_mode);
      break;
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
    case OPT_ALWAYS_ALLOW_IDTOKEN: arguments->always_allow_idtoken = 1; break;
    case 't':
      if (!isdigit(*arg)) {
        return ARGP_ERR_UNKNOWN;
      }
      arguments->lifetime.lifetime    = strToInt(arg);
      arguments->lifetime.argProvided = 1;
      break;
    case OPT_SECCOMP: arguments->seccomp = 1; break;
    case OPT_REMOTE: arguments->remote = 1; break;
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

struct argp argp = {options, parse_opt, args_doc, doc, 0, 0, 0};

void initArguments(struct arguments* arguments) {
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
  arguments->pw_file                 = NULL;
  arguments->pw_env                  = NULL;
  arguments->confirm                 = 0;
  arguments->always_allow_idtoken    = 0;
  arguments->remote                  = 0;
  arguments->pw_prompt_mode          = PROMPT_MODE_CLI;
  set_pw_prompt_mode(arguments->pw_prompt_mode);
}
