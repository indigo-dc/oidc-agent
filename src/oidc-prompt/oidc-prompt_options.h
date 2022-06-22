#ifndef OIDC_TOKEN_OPTIONS_H
#define OIDC_TOKEN_OPTIONS_H

#include <argp.h>

#include "wrapper/list.h"

struct arguments {
  char*   req_type;
  char*   title;
  char*   text;
  char*   label;
  int     timeout;
  char*   init;
  list_t* additional_args;
};

void initArguments(struct arguments* arguments);

extern struct argp argp;

#endif  // OIDC_TOKEN_OPTIONS_H
