#ifndef OIDC_JSON_H
#define OIDC_JSON_H

#include "key_value.h"
#include "oidc_error.h"
#include "wrapper/cjson.h"
#include "wrapper/list.h"

void _secFreeJson(cJSON* cjson);

char*        getJSONValue(const cJSON* cjson, const char* key);
char*        getJSONValueFromString(const char* json, const char* key);
oidc_error_t getJSONValues(const cJSON* cjson, struct key_value* pairs,
                           size_t size);
oidc_error_t getJSONValuesFromString(const char* json, struct key_value* pairs,
                                     size_t size);

int jsonHasKey(const cJSON* cjson, const char* key);
int jsonStringHasKey(const char* json, const char* key);
int isJSONObject(const char* json);
int isJSONArray(const char* json);
int jsonArrayIsEmpty(cJSON* json);

char*   jsonToString(cJSON* cjson);
char*   jsonToStringUnformatted(cJSON* cjson);
cJSON*  stringToJson(const char* json);
cJSON*  stringToJsonDontLogError(const char* json);
list_t* JSONArrayToList(const cJSON* cjson);
list_t* JSONArrayStringToList(const char* json);
char*   JSONArrayToDelimitedString(const cJSON* cjson, char* delim);
char*   JSONArrayStringToDelimitedString(const char* json, char* delim);
cJSON*  listToJSONArray(list_t* list);

cJSON*       generateJSONObject(const char* k1, int type1, const char* v1, ...);
oidc_error_t setJSONValue(cJSON* cjson, const char* key, const char* value);
oidc_error_t setJSONValueIfNotSet(cJSON* cjson, const char* key,
                                  const char* value);
cJSON*       jsonAddJSON(cJSON* cjson, const char* key, cJSON* item);
cJSON*       jsonAddObjectValue(cJSON* cjson, const char* key,
                                const char* json_object);
cJSON* jsonAddArrayValue(cJSON* cjson, const char* key, const char* json_array);
cJSON* jsonAddNumberValue(cJSON* cjson, const char* key, const double value);
cJSON* jsonAddStringValue(cJSON* cjson, const char* key, const char* value);
cJSON* jsonAddBoolValue(cJSON* cjson, const char* key,
                        const unsigned char value);
cJSON* jsonArrayAddStringValue(cJSON* cjson, const char* value);
cJSON* generateJSONArray(char* v1, ...);
cJSON* mergeJSONObjects(const cJSON* j1, const cJSON* j2);
char*  mergeJSONObjectStrings(const char* j1, const char* j2);
cJSON* jsonMergePatch(const cJSON* const target, const cJSON* const patch);
cJSON* appendArrayToArray(cJSON* array, const cJSON* appendIt);
cJSON* uniquifyArray(cJSON* array);

#ifndef secFreeJson
#define secFreeJson(ptr) \
  do {                   \
    _secFreeJson((ptr)); \
    (ptr) = NULL;        \
  } while (0)
#endif  // secFreeJson

#endif  // OIDC_JSON_H
