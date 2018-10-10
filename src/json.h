#ifndef OIDC_JSON_H
#define OIDC_JSON_H

#include "key_value.h"
#include "oidc_error.h"

#include "../lib/cJSON/cJSON.h"
#include "../lib/list/src/list.h"

char*  jsonToString(cJSON* cjson);
cJSON* stringToJson(const char* json);
void   secFreeJson(cJSON* cjson);

/** @fn char* getJSONValue(const cJSON* cjson, const char* key)
 * @brief returns the value for a given \p key
 * @param cjson the cJSON item
 * @param key the key
 * @return the value for the \p key. Has to be freed.
 */
char* getJSONValue(const cJSON* cjson, const char* key);

/** @fn char* getJSONValueFromString(const char* json, const char* key)
 * @brief returns the value from a json string for a given \p key
 * @param json a pointer to the json string
 * @param key the key
 * @return the value for the \p key. Has to be freed.
 */
char* getJSONValueFromString(const char* json, const char* key);

/** @fn oidc_error_t getJSONValues(const cJSON* cjson, struct key_value* pairs,
 * size_t size)
 * @brief gets multiple values from a cJSON item
 * @param cjson the cJSON item
 * @param pairs an array of key_value pairs. The keys are used as keys. A
 * pointer to the result is stored in the value field. The previous pointer is
 * not freed, thus it should be NULL.
 * @param size the number of key value pairs
 * return the number of set values or an error code on failure
 */
oidc_error_t getJSONValues(const cJSON* cjson, struct key_value* pairs,
                           size_t size);

/** @fn oidc_error_t getJSONValuesFromString(const char* json, struct key_value*
 * pairs, size_t size)
 * @brief gets multiple values from a json string
 * @param json the json string to be parsed
 * @param pairs an array of key_value pairs. The keys are used as keys. A
 * pointer to the result is stored in the value field. The previous pointer is
 * not freed, thus it should be NULL.
 * @param size the number of key value pairs
 * return the number of set values or an error code on failure
 */
oidc_error_t getJSONValuesFromString(const char* json, struct key_value* pairs,
                                     size_t size);

int     jsonHasKey(const cJSON* cjson, const char* key);
int     jsonStringHasKey(const char* json, const char* key);
int     isJSONObject(const char* json);
char*   jsonToString(cJSON* cjson);
cJSON*  stringToJson(const char* json);
list_t* JSONArrayToList(const cJSON* cjson);
list_t* JSONArrayStringToList(const char* json);
char*   JSONArrayToDelimitedString(const cJSON* cjson, char delim);
char*   JSONArrayStringToDelimitedString(const char* json, char delim);
char*   JSONArrayToDelimitedString(const cJSON* cjson, char delim);
cJSON*  jsonAddJSON(cJSON* cjson, const char* key, cJSON* item);
cJSON*  generateJSONObject(char* k1, char* v1, int isString1, ...);
cJSON*  jsonAddObjectValue(cJSON* cjson, const char* key,
                           const char* json_object);
cJSON* jsonAddArrayValue(cJSON* cjson, const char* key, const char* json_array);
cJSON* jsonAddNumberValue(cJSON* cjson, const char* key, const double value);
cJSON* jsonAddStringValue(cJSON* cjson, const char* key, const char* value);
cJSON* listToJSONArray(list_t* list);
cJSON* generateJSONArray(char* v1, ...);

// char* json_addValue(char* json, const char* key, const char* value);
// char* json_addStringValue(char* json, const char* key, const char* value);
// char* json_arrAdd(char* json, const char* value);
// int          isJSONObject(const char* json);
// char*        generateJSONArray(char* v1, ...);
// list_t*      getKeysfromTokens(jsmntok_t t[], int r, const char* json,
//                                int strHasToBeValid);
// list_t*      getJSONKeys(const char* json, int strHasToBeValid);
// char*        mergeJSONObject(const char* j1, const char* j2);

#endif  // OIDC_JSON_H
