#ifndef OIDCAGENT_INTERNAL_PARSER_H
#define OIDCAGENT_INTERNAL_PARSER_H

#include "utils/oidc_error.h"

char*        parseForConfig(char* res);
char*        parseForInfo(char* res);
oidc_error_t parseForErrorCode(char* res);
char*        parseStateLookupRes(char* res);

#endif  // OIDCAGENT_INTERNAL_PARSER_H
