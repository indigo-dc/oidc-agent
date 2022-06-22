#include "oidc-prompt_options.h"

#include "utils/listUtils.h"
#include "utils/memory.h"
#include "utils/string/stringUtils.h"

static struct argp_option options[] = {{0, 0, 0, 0, "Help:", -1},
                                       {0, 'h', 0, OPTION_HIDDEN, 0, -1},
                                       {0, 0, 0, 0, 0, 0}};

static error_t parse_opt(int key, char* arg, struct argp_state* state) {
  struct arguments* arguments = state->input;

  switch (key) {
    case 'h':
      argp_state_help(state, state->out_stream, ARGP_HELP_STD_HELP);
      break;
    case ARGP_KEY_ARG:
      switch (state->arg_num) {
        case 0: arguments->req_type = arg; break;
        case 1: arguments->title = arg; break;
        case 2: arguments->text = arg; break;
        case 3: arguments->label = arg; break;
        case 4: arguments->timeout = strToInt(arg); break;
        case 5: arguments->init = arg; break;
        default:
          if (arguments->additional_args == NULL) {
            arguments->additional_args = list_new();
          }
          list_rpush(arguments->additional_args, list_node_new(arg));
      }
      break;
    case ARGP_KEY_END:
      if (state->arg_num < 3) {
        argp_usage(state);
      }
      break;
    default: return ARGP_ERR_UNKNOWN;
  }
  return 0;
}

static char args_doc[] =
    "TYPE TITLE TEXT [LABEL [TIMEOUT [INIT [LIST_ELEMENTS ...]]]]";

static char doc[] =
    "oidc-prompt -- A dialog tool for prompting the user.\n\n"
    "This tool is intended as an internal tool of oidc-agent. Different "
    "oidc-agent components might use it to prompt the user for information. "
    "The user or other applications should not call oidc-prompt directly.";

struct argp argp = {options, parse_opt, args_doc, doc, 0, 0, 0};

void initArguments(struct arguments* arguments) {
  arguments->req_type        = NULL;
  arguments->title           = NULL;
  arguments->text            = NULL;
  arguments->label           = NULL;
  arguments->timeout         = 0;
  arguments->init            = NULL;
  arguments->additional_args = NULL;
}
