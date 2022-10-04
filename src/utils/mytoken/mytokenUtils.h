#ifndef OIDC_AGENT_MYTOKENUTILS_H
#define OIDC_AGENT_MYTOKENUTILS_H

#include "wrapper/cjson.h"

cJSON* createFinalTemplate(const cJSON* content,
                           cJSON* (*readFnc)(const char*));
cJSON* parseProfile(const cJSON* content);
cJSON* parseCapabilityTemplate(const cJSON* content);
cJSON* parseRestrictionsTemplate(const cJSON* content);
cJSON* parseRotationTemplate(const cJSON* content);

#endif  // OIDC_AGENT_MYTOKENUTILS_H
