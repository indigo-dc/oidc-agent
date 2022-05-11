#ifndef OIDC_AGENT_MYTOKENUTILS_H
#define OIDC_AGENT_MYTOKENUTILS_H

#include "wrapper/cjson.h"

cJSON* createFinalTemplate(const cJSON* content,
                           cJSON* (*readFnc)(const char*));

#endif  // OIDC_AGENT_MYTOKENUTILS_H
