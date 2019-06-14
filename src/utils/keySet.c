#include "keySet.h"
#include "utils/json.h"
#include "utils/memory.h"

void _secFreeKeySetSEstr(struct keySetSEstr keys) {
  if (keys.sign != keys.enc) {
    secFree(keys.enc);
  }
  secFree(keys.sign);
}

void _secFreeKeySetPPstr(struct keySetPPstr keys) {
  if (keys.pub != keys.priv) {
    secFree(keys.priv);
  }
  secFree(keys.pub);
}

cJSON* _keySetToJSON(const char* k1, const char* k2) {
  char*  arrayStr = oidc_sprintf("[%s,%s]", k1, k2);
  cJSON* json     = generateJSONObject("keys", cJSON_Array, arrayStr, NULL);
  secFree(arrayStr);
  return json;
}

cJSON* keySetSEToJSON(const struct keySetSEstr keys) {
  return _keySetToJSON(keys.sign, keys.enc);
}

char* keySetSEToJSONString(const struct keySetSEstr keys) {
  cJSON* json = keySetSEToJSON(keys);
  if (json == NULL) {
    return NULL;
  }
  char* ret = jsonToString(json);
  secFreeJson(json);
  return ret;
}
