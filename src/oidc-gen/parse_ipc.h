#ifndef PARSE_IPC_GEN_H
#define PARSE_IPC_GEN_H

#include "oidc-gen/oidc-gen_options.h"
char* gen_parseResponse(char* res, const struct arguments* arguments,
                        const char* suggested_password);

#endif  // PARSE_IPC_GEN_H
