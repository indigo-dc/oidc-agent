#include "oidc-agent_options.h"

#include "utils/agentLogger.h"
#include "utils/config/agent_config.h"
#include "utils/listUtils.h"
#include "utils/string/stringUtils.h"

#define OPT_NOAUTOLOAD 2
#define OPT_NO_WEBSERVER 3
#define OPT_GROUP 5
#define OPT_NO_SCHEME 6
#define OPT_LOG_CONSOLE 7
#define OPT_ALWAYS_ALLOW_IDTOKEN 8
#define OPT_STATUS 9
#define OPT_JSON 10
#define OPT_QUIET 11
#define OPT_NO_AUTOREAUTHENTICATE 12

void initArguments(struct arguments* arguments) {
  arguments->kill_flag             = 0;
  arguments->console               = 0;
  arguments->debug                 = getAgentConfig()->debug;
  arguments->lifetime              = getAgentConfig()->lifetime;
  arguments->no_autoload           = !getAgentConfig()->autoload;
  arguments->confirm               = getAgentConfig()->confirm;
  arguments->no_webserver          = !getAgentConfig()->webserver;
  arguments->group                 = getAgentConfig()->group;
  arguments->socket_path           = getAgentConfig()->bind_address;
  arguments->no_scheme             = !getAgentConfig()->customurischeme;
  arguments->always_allow_idtoken  = getAgentConfig()->alwaysallowidtoken;
  arguments->log_console           = 0;
  arguments->status                = 0;
  arguments->json                  = 0;
  arguments->quiet                 = 0;
  arguments->no_autoreauthenticate = !getAgentConfig()->autoreauth;
  arguments->command               = NULL;
  arguments->args_list             = NULL;
}

static struct argp_option options[] = {
    {0, 0, 0, 0, "General:", 1},
    {"kill", 'k', 0, 0,
     "Kill the current agent (given by the OIDCD_PID environment variable)", 1},
    {"lifetime", 't', "TIME", 0,
     "Sets a default value in seconds for the maximum lifetime of account "
     "configurations added to the agent. A lifetime specified for an account "
     "configuration with oidc-add overwrites this default value. Without this "
     "option the default maximum lifetime is forever.",
     1},
    {"no-autoload", OPT_NOAUTOLOAD, 0, 0,
     "Disables the autoload feature: A token request cannot load the needed "
     "configuration. You have to do it with oidc-add.",
     1},
    {"no-autoreauthenticate", OPT_NO_AUTOREAUTHENTICATE, 0, 0,
     "Disables the automatic re-authentication feature: If a refresh token "
     "expired the re-atuhentiacte is not started automatically; you have to do "
     "it manually.",
     1},
    {"no-auto-reauthenticate", OPT_NO_AUTOREAUTHENTICATE, 0, OPTION_ALIAS, NULL,
     1},
    {"confirm", 'c', 0, 0,
     "Requires user confirmation when an application requests an access token "
     "for any loaded configuration",
     1},
    {"no-webserver", OPT_NO_WEBSERVER, 0, 0,
     "This option applies only when the "
     "authorization code flow is used. oidc-agent will not start a webserver. "
     "Redirection to oidc-gen through a custom uri scheme redirect uri and "
     "'manual' redirect is possible.",
     1},
    {"no-scheme", OPT_NO_SCHEME, 0, 0,
     "This option applies only when the "
     "authorization code flow is used. oidc-agent will not use a custom uri "
     "scheme redirect.",
     1},
    {"with-group", OPT_GROUP, "GROUP_NAME", OPTION_ARG_OPTIONAL,
     "This option allows that applications running under another user can "
     "access the agent. The user running the other application and the user "
     "running the agent have to be in the specified group. If no GROUP_NAME is "
     "specified the default is 'oidc-agent'.",
     1},
    {"socket-path", 'a', "PATH", 0,
     "Create the UNIX-domain used for communicating with the agent at this "
     "PATH. The default is '$TMPDIR/oidc-XXXXXX/oidc-agent.<ppid>'. Use "
     "'XXXXXX' as the last six characters of a directory in the path to "
     "substitute them with random characters.",
     1},
    {"bind-address", 'a', "PATH", OPTION_ALIAS, NULL, 1},
    {"always-allow-idtoken", OPT_ALWAYS_ALLOW_IDTOKEN, 0, 0,
     "Always allow id-token requests without manual approval by the user.", 1},
    {"json", OPT_JSON, 0, 0,
     "Print agent socket and pid as JSON instead of bash.", 1},
    {"quiet", OPT_QUIET, 0, 0, "Disable informational messages to stdout.", 1},
    {0, 0, 0, 0, "Verbosity:", 2},
    {"debug", 'g', 0, 0, "Sets the log level to DEBUG.", 2},
    {"console", 'd', 0, 0,
     "Runs oidc-agent on the console, without daemonizing.", 2},
    {"log-stderr", OPT_LOG_CONSOLE, 0, 0,
     "Additionally prints log messages to stderr.", 2},
    {"status", OPT_STATUS, 0, 0,
     "Connects to the currently running agent and prints status information "
     "about it.",
     2},
    {0, 0, 0, 0, "Help:", -1},
    {0, 'h', 0, OPTION_HIDDEN, 0, -1},
    {0, 0, 0, 0, 0, 0}};

static char args_doc[] = "";
static char doc[]      = "oidc-agent -- An agent to manage oidc token";

static error_t parse_opt(int key, char* arg __attribute__((unused)),
                         struct argp_state* state) {
  struct arguments* arguments = state->input;
  switch (key) {
    case 'k': arguments->kill_flag = 1; break;
    case 'g': arguments->debug = 1; break;
    case 'd': arguments->console = 1; break;
    case 'c': arguments->confirm = 1; break;
    case OPT_NOAUTOLOAD: arguments->no_autoload = 1; break;
    case OPT_NO_WEBSERVER: arguments->no_webserver = 1; break;
    case OPT_NO_SCHEME: arguments->no_scheme = 1; break;
    case OPT_GROUP: arguments->group = arg ?: "oidc-agent"; break;
    case 'a': arguments->socket_path = arg; break;
    case OPT_LOG_CONSOLE:
      arguments->log_console = 1;
      setLogWithTerminal();
      break;
    case OPT_ALWAYS_ALLOW_IDTOKEN: arguments->always_allow_idtoken = 1; break;
    case OPT_STATUS: arguments->status = 1; break;
    case 't':
      if (!isdigit(*arg)) {
        return ARGP_ERR_UNKNOWN;
      }
      arguments->lifetime = strToInt(arg);
      break;
    case OPT_JSON: arguments->json = 1; break;
    case OPT_QUIET: arguments->quiet = 1; break;
    case OPT_NO_AUTOREAUTHENTICATE: arguments->no_autoreauthenticate = 1; break;
    case 'h':
      argp_state_help(state, state->out_stream, ARGP_HELP_STD_HELP);
      break;
    case ARGP_KEY_ARG:
      if (state->arg_num == 0) {
        arguments->command   = arg;
        arguments->args_list = list_new();
      }
      list_rpush(arguments->args_list, list_node_new(arg));
      break;
    case ARGP_KEY_END:
      arguments->args = (char* const*)listToArray(arguments->args_list);
      break;
    default: return ARGP_ERR_UNKNOWN;
  }
  return 0;
}

struct argp argp = {options, parse_opt, args_doc, doc, 0, 0, 0};
