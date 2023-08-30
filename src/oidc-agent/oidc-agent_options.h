#ifndef OIDC_AGENT_OPTIONS_H
#define OIDC_AGENT_OPTIONS_H

#include <argp.h>

#include "utils/lifetimeArg.h"
#include "wrapper/list.h"

struct arguments {
  unsigned char kill_flag;
  unsigned char debug;
  unsigned char console;
  unsigned char no_autoload;
  unsigned char confirm;
  unsigned char no_webserver;
  unsigned char no_scheme;
  unsigned char always_allow_idtoken;
  unsigned char log_console;
  unsigned char status;
  unsigned char json;
  unsigned char quiet;
  unsigned char no_autoreauthenticate;

  time_t lifetime;

  const char* group;
  const char* socket_path;

  const char*  command;
  list_t*      args_list;
  char* const* args;
};

void initArguments(struct arguments* arguments);

extern struct argp argp;

#endif  // OIDC_AGENT_OPTIONS_H
