#ifndef OIDC_ADD_OPTIONS_H
#define OIDC_ADD_OPTIONS_H

#include "utils/lifetimeArg.h"

#include <argp.h>

#define ARG_PROVIDED_BUT_USES_DEFAULT 2

struct arguments {
  char* args[1]; /* account */
  char* pw_cmd;
  char* pw_file;
  char* pw_env;

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
  unsigned char always_allow_idtoken;
  unsigned char pw_prompt_mode;
  unsigned char remote;

  struct lifetimeArg pw_lifetime;
  struct lifetimeArg lifetime;
};

void initArguments(struct arguments* arguments);

extern struct argp argp;

#endif  // OIDC_ADD_OPTIONS_H
