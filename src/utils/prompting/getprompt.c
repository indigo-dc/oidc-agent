#include "getprompt.h"

#include "utils/json.h"
#include "utils/oidc_error.h"
#include "utils/prompting/templates/templates.h"
#include "utils/string/stringUtils.h"
#include "wrapper/mustache.h"

char* getprompt(const char* template, cJSON* data) {
  char*  result    = NULL;
  size_t resultLen = 0;
  int    ret = mustach_cJSON_mem(template, 0, data, 0, &result, &resultLen);
  if (ret != 0) {
    oidc_setErrnoError();
    return NULL;
  }
  char* str = oidc_strcopy(result);
  free(result);
  return str;
}