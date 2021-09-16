#ifndef OIDC_TOKEN_H
#define OIDC_TOKEN_H

#include "api.h"
#include "defines/version.h"
#include "oidc-token_options.h"

const char* argp_program_version = TOKEN_VERSION;

const char* argp_program_bug_address = BUG_ADDRESS;

void printEnvCommands(struct arguments*     arguments,
                      struct token_response response);
#endif  // OIDC_TOKEN_H
