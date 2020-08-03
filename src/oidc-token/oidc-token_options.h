#ifndef OIDC_TOKEN_OPTIONS_H
#define OIDC_TOKEN_OPTIONS_H

#include "list/list.h"

#include <argp.h>
#include <time.h>

#define ENV_TOKEN "OIDC_AT"
#define ENV_ISS "OIDC_ISS"
#define ENV_EXP "OIDC_EXP"

struct optional_arg {
  char* str;
  short useIt;
};

struct arguments {
  char* args[1]; /* account shortname */

  list_t* scopes;

  char* application_name;
  char* audience;

  struct optional_arg issuer_env;
  struct optional_arg expiration_env;
  struct optional_arg token_env;

  unsigned char seccomp;
  unsigned char printAll;
  unsigned char idtoken;
  unsigned char forceNewToken;

  time_t min_valid_period;
};

void initArguments(struct arguments* arguments);

extern struct argp argp;

#endif  // OIDC_TOKEN_OPTIONS_H
