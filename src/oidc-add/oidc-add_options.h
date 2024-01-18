#ifndef OIDC_ADD_OPTIONS_H
#define OIDC_ADD_OPTIONS_H

#include <argp.h>

#include "utils/lifetimeArg.h"

#define ARG_PROVIDED_BUT_USES_DEFAULT 2

struct arguments {
  char* args[1]; /* account */
  char* pw_cmd;
  char* pw_file;
  char* pw_env;

  unsigned char remove : 1;
  unsigned char removeAll : 1;
  unsigned char debug : 1;
  unsigned char verbose : 1;
  unsigned char listConfigured : 1;
  unsigned char listLoaded : 1;
  unsigned char print : 1;
  unsigned char lock : 1;
  unsigned char unlock : 1;
  unsigned char confirm : 1;
  unsigned char always_allow_idtoken : 1;
  unsigned char pw_prompt_mode : 2;
  unsigned char remote : 1;
  unsigned char force : 1;
  unsigned char plainadd : 1;

  struct lifetimeArg pw_lifetime;
  struct lifetimeArg lifetime;
};

void initArguments(struct arguments* arguments);

extern struct argp argp;

#endif  // OIDC_ADD_OPTIONS_H
