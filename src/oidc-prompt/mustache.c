
#include "html/templates.h"
#include "mustache-wrapper.h"
#include "utils/json.h"
#include "utils/oidc_error.h"
#include "utils/string/stringUtils.h"

char* mustache(const char* layout, const char* template, const cJSON* data) {
  cJSON* root      = mergeJSONObjects(data, partials_json());
  char*  result    = NULL;
  size_t resultLen = 0;
  int    ret = mustach_cJSON_mem(template, 0, root, 0, &result, &resultLen);
  if (ret != 0) {
    oidc_setErrnoError();
    secFreeJson(root);
    return NULL;
  }
  if (layout == NULL) {
    secFreeJson(root);
    char* str = oidc_strcopy(result);
    free(result);
    return str;
  }
  root = jsonAddStringValue(root, "content", result);
  free(result);
  result = NULL;
  ret    = mustach_cJSON_mem(layout, 0, root, 0, &result, &resultLen);
  secFreeJson(root);
  if (ret != 0) {
    oidc_setErrnoError();
    return NULL;
  }

  char* str = oidc_strcopy(result);
  free(result);
  return str;
}

char* mustache_main(const char* template, const cJSON* data) {
  return mustache(LAYOUT_MAIN, template, data);
}
