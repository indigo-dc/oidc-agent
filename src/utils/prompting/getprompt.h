#ifndef OIDC_GETTEXT_H
#define OIDC_GETTEXT_H

#include "prompt_mode.h"
#include "templates/templates.h"
#include "wrapper/cjson.h"

#define PROMPTTEMPLATE(MSGID)                                     \
  prompt_mode() == PROMPT_MODE_GUI ? PROMPTTEMPLATE_##MSGID##_GUI \
                                   : PROMPTTEMPLATE_##MSGID##_CLI

char* getprompt(const char* template, cJSON* data);

#endif  // OIDC_GETTEXT_H
