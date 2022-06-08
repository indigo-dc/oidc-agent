#ifndef OIDC_TOKEN_OPTIONS_H
#define OIDC_TOKEN_OPTIONS_H

#include <argp.h>
#include <time.h>

#include "wrapper/list.h"

#define ENV_TOKEN "OIDC_AT"
#define ENV_ISS "OIDC_ISS"
#define ENV_EXP "OIDC_EXP"

struct optional_arg {
  char* str;
  short useIt;
};

struct arguments {
  char* args[1]; /* account shortname */

  char* scopes;
  char* application_name;
  char* audience;

  struct optional_arg issuer_env;
  struct optional_arg expiration_env;
  struct optional_arg token_env;

  struct optional_arg mytoken;

  unsigned char printAll;
  unsigned char idtoken;
  unsigned char forceNewToken;

  time_t min_valid_period;
};

void initArguments(struct arguments* arguments);

extern struct argp argp;

#endif  // OIDC_TOKEN_OPTIONS_H
