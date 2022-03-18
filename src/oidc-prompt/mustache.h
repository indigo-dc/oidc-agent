#ifndef OIDC_PROMPT_MUSTACHE_H
#define OIDC_PROMPT_MUSTACHE_H

#include "html/templates.h"
#include "wrapper/cjson.h"

char* mustache_main(const char* template, const cJSON* data);
char* mustache(const char* layout, const char* template, const cJSON* data);

#endif  // OIDC_PROMPT_MUSTACHE_H
