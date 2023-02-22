#include "profile.h"

#include <stddef.h>

#include "utils/json.h"

cJSON* parseUsedMytokenProfile(const char* profile) {
  if (profile == NULL) {
    return cJSON_CreateObject();
  }
  cJSON* json = cJSON_Parse(profile);
  if (json != NULL && cJSON_IsObject(json)) {
    return json;
  }
  secFreeJson(json);
  json = cJSON_CreateObject();
  setJSONValue(json, "include", profile);
  return json;
}