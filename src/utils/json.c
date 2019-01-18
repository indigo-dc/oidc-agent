#include "json.h"

#include "listUtils.h"
#include "oidc_error.h"
#include "pass.h"
#include "stringUtils.h"

#include <stdarg.h>
#include <syslog.h>

static cJSON_Hooks hooks;
static int         jsonInitDone = 0;

/**
 * @brief initializes the cJSON memory allocator and deallocator if not done yet
 * @internal
 */
void initCJSON() {
  if (!jsonInitDone) {
    hooks.malloc_fn = secAlloc;
    hooks.free_fn   = _secFree;
    cJSON_InitHooks(&hooks);
    jsonInitDone = 1;
  }
}

/**
 * @brief converts a cJSON object into a string
 * @param cjson the cJSON object to be converted
 * @return a pointer to a string representation of @p cjson. Has to be freed
 * after usage.
 */
char* jsonToString(cJSON* cjson) {
  if (cjson == NULL) {
    oidc_setArgNullFuncError(__func__);
    return NULL;
  }
  initCJSON();
  return cJSON_Print(cjson);
}

char* jsonToStringUnformatted(cJSON* cjson) {
  char* json = jsonToString(cjson);
  cJSON_Minify(json);
  return json;
}

/**
 * @brief parses a string into an cJSON object
 * @param json the json string
 * @param logError if @c 0 errors are not logged, otherwise error are logged
 * @return a pointer to a cJSON object. Has to be freed after usage.
 * @internal
 */
cJSON* _stringToJson(const char* json, int logError) {
  if (NULL == json) {
    oidc_setArgNullFuncError(__func__);
    return NULL;
  }
  initCJSON();
  char* minJson = oidc_strcopy(json);
  cJSON_Minify(minJson);
  syslog(LOG_AUTHPRIV | LOG_DEBUG, "Parsing json '%s'", minJson);
  cJSON* cj = cJSON_Parse(minJson);
  if (cj == NULL) {
    oidc_errno = OIDC_EJSONPARS;
    if (logError) {
      syslog(LOG_AUTHPRIV | LOG_ERR, "Parsing failed somewhere around %s",
             cJSON_GetErrorPtr());
    }
  }
  secFree(minJson);
  return cj;
}

/**
 * @brief parses a string into an cJSON object
 * @param json the json string
 * @return a pointer to a cJSON object. Has to be freed after usage.
 * @note this function logs parsing error
 */
cJSON* stringToJson(const char* json) { return _stringToJson(json, 1); }

/**
 * @brief parses a string into an cJSON object
 * @param json the json string
 * @return a pointer to a cJSON object. Has to be freed after usage.
 * @note this function does not log parsing error and is mainly used for
 * checking if a string would parse correctly into a cJSON object
 */
cJSON* stringToJsonDontLogError(const char* json) {
  return _stringToJson(json, 0);
}

/**
 * @brief checks if a string represents a json object
 * @param json the (possibly) json string
 * @return @c 1 if @p json holds a json object, @c 0 if not
 */
int isJSONObject(const char* json) {
  if (NULL == json) {
    oidc_setArgNullFuncError(__func__);
    return 0;
  }
  initCJSON();
  cJSON* cj = stringToJsonDontLogError(json);
  if (cj == NULL) {
    return 0;
  }
  int res = cJSON_IsObject(cj);
  cJSON_Delete(cj);
  return res;
}

/**
 * @brief safly calls cJSON_Delete freeing the cJSON Object
 * @param cjson the cJSON Object to be freed
 */
void _secFreeJson(cJSON* cjson) {
  if (cjson == NULL) {
    return;
  }
  initCJSON();
  cJSON_Delete(cjson);
}

/**
 * @brief checks if a json string contains a specific key
 * @param json a string representing a json object
 * @param key the string that might be contained as a key
 * @return @c 1 if @p json contains @p key; @c 0 otherwise
 * @note if you want to check multiple keys or you might do some other json
 * operation, you probably should first parse @p json into an cJSON object and
 * then use @c jsonHasKey
 */
int jsonStringHasKey(const char* json, const char* key) {
  if (NULL == json || NULL == key) {
    oidc_setArgNullFuncError(__func__);
    return 0;
  }
  initCJSON();
  cJSON* cj = stringToJson(json);
  if (cj == NULL) {
    return 0;
  }
  int res = jsonHasKey(cj, key);
  secFreeJson(cj);
  return res;
}

/**
 * @brief checks if a cSJON object contains a specific key
 * @param cjson the cJSON object
 * @param key the string that might be contained as a key
 * @return @c 1 if @p cjson contains @p key; @c 0 otherwise
 */
int jsonHasKey(const cJSON* cjson, const char* key) {
  if (NULL == cjson || NULL == key) {
    oidc_setArgNullFuncError(__func__);
    return 0;
  }
  initCJSON();
  char* value = getJSONValue(cjson, key);
  if (strValid(value)) {
    secFree(value);
    return 1;
  } else {
    secFree(value);
    return 0;
  }
}

/**
 * @brief gets the value field of a cJSON object (represented as string)
 * @param valueItem the cJSON object
 * @return a pointer to a string holding the value. Has to be freed after usage.
 */
char* getJSONItemValue(cJSON* valueItem) {
  if (NULL == valueItem) {
    oidc_setArgNullFuncError(__func__);
    return NULL;
  }
  initCJSON();
  if (cJSON_IsString(valueItem)) {
    return oidc_strcopy(cJSON_GetStringValue(valueItem));
  }
  return cJSON_Print(valueItem);
}

/**
 * @brief gets the value for a given key from a cJSON object
 * @param cjson the cJSON object
 * @param key the key
 * @return a pointer to a string holding the value for the \p key. Has to be
 * freed after usage.
 */
char* getJSONValue(const cJSON* cjson, const char* key) {
  if (NULL == cjson || NULL == key) {
    oidc_setArgNullFuncError(__func__);
    return NULL;
  }
  initCJSON();
  // syslog(LOG_AUTHPRIV | LOG_DEBUG, "Getting value for key '%s'", key);
  if (!cJSON_IsObject(cjson)) {
    oidc_errno = OIDC_EJSONOBJ;
    return NULL;
  }
  if (!cJSON_HasObjectItem(cjson, key)) {
    oidc_errno = OIDC_EJSONNOFOUND;
    return NULL;
  }
  cJSON* valueItem = cJSON_GetObjectItemCaseSensitive(cjson, key);
  char*  value     = getJSONItemValue(valueItem);
  // syslog(LOG_AUTHPRIV | LOG_DEBUG, "value for key '%s' is '%s'", key, value);
  return value;
}

/**
 * @brief gets the value for a given key from a json string
 * @param json a pointer to the json string
 * @param key the key
 * @return a pointer to a string holding the value for the \p key. Has to be
 * freed after usage.
 */
char* getJSONValueFromString(const char* json, const char* key) {
  if (NULL == json || NULL == key) {
    oidc_setArgNullFuncError(__func__);
    return NULL;
  }
  initCJSON();
  cJSON* cj = stringToJson(json);
  if (cj == NULL) {
    return NULL;
  }
  char* value = getJSONValue(cj, key);
  secFreeJson(cj);
  return value;
}

/**
 * @brief gets multiple values from a cJSON object
 * @param cjson the cJSON object
 * @param pairs an array of key_value pairs. The keys are used as keys. A
 * pointer to the result is stored in the value field. The previous pointer is
 * not freed, thus it should be NULL.
 * @param size the number of key value pairs
 * @return the number of set values or an error code on failure
 */
oidc_error_t getJSONValues(const cJSON* cjson, struct key_value* pairs,
                           size_t size) {
  if (NULL == cjson || NULL == pairs || size == 0) {
    oidc_setArgNullFuncError(__func__);
    return oidc_errno;
  }
  initCJSON();
  if (!cJSON_IsObject(cjson)) {
    oidc_errno = OIDC_EJSONOBJ;
    return oidc_errno;
  }
  unsigned int i;
  for (i = 0; i < size; i++) {
    pairs[i].value = getJSONValue(cjson, pairs[i].key);
  }
  return i;
}

/**
 * @brief gets multiple values from a json string
 * @param json the json string to be parsed
 * @param pairs an array of key_value pairs. The keys are used as keys. A
 * pointer to the result is stored in the value field. The previous pointer is
 * not freed, thus it should be NULL.
 * @param size the number of key value pairs
 * @return the number of set values or an error code on failure
 */
oidc_error_t getJSONValuesFromString(const char* json, struct key_value* pairs,
                                     size_t size) {
  if (NULL == json || NULL == pairs || size == 0) {
    oidc_setArgNullFuncError(__func__);
    return oidc_errno;
  }
  initCJSON();
  cJSON* cj = stringToJson(json);
  if (cj == NULL) {
    return oidc_errno;
  }
  oidc_error_t e = getJSONValues(cj, pairs, size);
  secFreeJson(cj);
  return e;
}

/**
 * @brief converts a cJSON JSONArray into a list
 * @param cjson the cJSON JSONArray
 * @return a pointer to a list. The list has to be freed after usage using
 * @c secFreeList.
 */
list_t* JSONArrayToList(const cJSON* cjson) {
  if (NULL == cjson) {
    oidc_setArgNullFuncError(__func__);
    return NULL;
  }
  initCJSON();
  if (!cJSON_IsArray(cjson)) {
    oidc_errno = OIDC_EJSONARR;
    return NULL;
  }

  int     j;
  list_t* l = list_new();
  l->free   = _secFree;
  l->match  = (int (*)(void*, void*)) & strequal;
  for (j = 0; j < cJSON_GetArraySize(cjson); j++) {
    list_rpush(l,
               list_node_new(getJSONItemValue(cJSON_GetArrayItem(cjson, j))));
  }
  return l;
}

/**
 * @brief converts a JSONArray string into a list
 * @param json a pointer to a string holding a JSONArray
 * @return a pointer to a list. The list has to be freed after usage using
 * @c secFreeList.
 */
list_t* JSONArrayStringToList(const char* json) {
  if (NULL == json) {
    oidc_setArgNullFuncError(__func__);
    return NULL;
  }
  initCJSON();
  cJSON* cj = stringToJson(json);
  if (cj == NULL) {
    return NULL;
  }
  list_t* l = JSONArrayToList(cj);
  secFreeJson(cj);
  return l;
}

/**
 * @brief converts a cJSON JSONArray into a string with specified delimiter
 * @param cjson the cJSON JSONArray
 * @param delim the delimiting character to be used
 * @return a pointer to a string holding the delimited lsit items. Has to be
 * freed after usage.
 */
char* JSONArrayToDelimitedString(const cJSON* cjson, char delim) {
  if (NULL == cjson) {
    oidc_setArgNullFuncError(__func__);
    return NULL;
  }
  initCJSON();
  list_t* list = JSONArrayToList(cjson);
  char*   str  = listToDelimitedString(list, delim);
  list_destroy(list);
  return str;
}

/**
 * @brief converts a JSONArray string into a string with specified delimiter
 * @param json a pointer to a string holding a JSONArray
 * @param delim the delimiting character to be used
 * @return a pointer to a string holding the delimited lsit items. Has to be
 * freed after usage.
 */
char* JSONArrayStringToDelimitedString(const char* json, char delim) {
  if (NULL == json) {
    oidc_setArgNullFuncError(__func__);
    return NULL;
  }
  initCJSON();
  cJSON* cj = stringToJson(json);
  if (cj == NULL) {
    return NULL;
  }
  char* str = JSONArrayToDelimitedString(cj, delim);
  secFreeJson(cj);
  return str;
}

cJSON* jsonAddStringValue(cJSON* cjson, const char* key, const char* value) {
  if (NULL == cjson || NULL == key || NULL == value) {
    oidc_setArgNullFuncError(__func__);
    return NULL;
  }
  initCJSON();
  cJSON_AddStringToObject(cjson, key, value);
  return cjson;
}
cJSON* jsonAddNumberValue(cJSON* cjson, const char* key, const double value) {
  if (NULL == cjson || NULL == key) {
    oidc_setArgNullFuncError(__func__);
    return NULL;
  }
  initCJSON();
  cJSON_AddNumberToObject(cjson, key, value);
  return cjson;
}
cJSON* jsonAddArrayValue(cJSON* cjson, const char* key,
                         const char* json_array) {
  if (NULL == cjson || NULL == key || NULL == json_array) {
    oidc_setArgNullFuncError(__func__);
    return NULL;
  }
  initCJSON();
  cJSON* array = stringToJson(json_array);
  cJSON_AddItemToObject(cjson, key, array);
  return cjson;
}
cJSON* jsonAddObjectValue(cJSON* cjson, const char* key,
                          const char* json_object) {
  if (NULL == cjson || NULL == key || NULL == json_object) {
    oidc_setArgNullFuncError(__func__);
    return NULL;
  }
  initCJSON();
  cJSON* object = stringToJson(json_object);
  cJSON_AddItemToObject(cjson, key, object);
  return cjson;
}
cJSON* jsonAddJSON(cJSON* cjson, const char* key, cJSON* item) {
  if (NULL == cjson || NULL == key || NULL == item) {
    oidc_setArgNullFuncError(__func__);
    return NULL;
  }
  initCJSON();
  cJSON_AddItemToObject(cjson, key, item);
  return cjson;
}

cJSON* jsonArrayAddStringValue(cJSON* cjson, const char* value) {
  if (value == NULL) {
    oidc_setArgNullFuncError(__func__);
    return NULL;
  }
  if (cjson == NULL) {
    cjson = cJSON_CreateArray();
  }
  cJSON_AddItemToArray(cjson, cJSON_CreateString(value));
  return cjson;
}

/**
 * @brief converts a list into a cJSON JSONArray
 * @param list a pointer to the list to be converted
 * @return a pointer to a cJSON JSONArray. Has to be freed after usage using
 * @c secFreeJson
 */
cJSON* listToJSONArray(list_t* list) {
  if (list == NULL) {
    oidc_setArgNullFuncError(__func__);
    return NULL;
  }
  initCJSON();
  cJSON* json = cJSON_CreateArray();
  if (json == NULL) {
    oidc_seterror("Coud not create json array");
    return NULL;
  }
  list_node_t*     node;
  list_iterator_t* it = list_iterator_new(list, LIST_HEAD);
  while ((node = list_iterator_next(it))) {
    cJSON_AddItemToArray(json, cJSON_CreateString(node->val));
  }
  list_iterator_destroy(it);
  return json;
}

/**
 * @brief Generates a cJSON JSONObject from key, type, value tuples
 * @param k1 the key for the first element
 * @param type1 the type for the first element. Has to be a cJSON Type, e.g.
 * @c cJSON_String
 * @param v1 the value for the first element. This cannot be a number.
 * @param params additional parameters can be specified, but only in tuples of
 * three: key, type, value. Value has to be either a @c char* or a number (no
 * floating point numbers supported yet)
 * @note the last argument MUST be @c NULL, otherwise behavior is unspecified
 * @return a pointer to a cJSON JSONObject. Has to be freed after usage using
 * @c secFsecFreeJson
 */
cJSON* generateJSONObject(char* k1, int type1, char* v1, ...) {
  initCJSON();
  syslog(LOG_AUTHPRIV | LOG_DEBUG, "Generating JSONObject");
  va_list args;
  va_start(args, v1);
  // syslog(LOG_AUTHPRIV | LOG_DEBUG, "First key:value is %s:%s", k1, v1);
  cJSON* json = cJSON_CreateObject();
  if (json == NULL) {
    oidc_seterror("Coud not create json object");
    return NULL;
  }
  char* key         = k1;
  char* value       = v1;
  int   type        = type1;
  long  numbervalue = 0;
  do {
    // syslog(LOG_AUTHPRIV | LOG_DEBUG, "key:value is %s:%s", key, value);
    cJSON* (*addFunc)(cJSON*, const char*, const char*);
    int useNumberAdd = 0;
    switch (type) {
      case cJSON_String: addFunc = jsonAddStringValue; break;
      case cJSON_Object: addFunc = jsonAddObjectValue; break;
      case cJSON_Array: addFunc = jsonAddArrayValue; break;
      case cJSON_Number: useNumberAdd = 1; break;
      default:
        syslog(LOG_AUTHPRIV | LOG_ERR, "unknown type %d", type);
        oidc_errno = OIDC_EJSONTYPE;
        return NULL;
    }
    if (useNumberAdd == 0) {
      json = addFunc(json, key, value);
    } else {
      json = jsonAddNumberValue(json, key, numbervalue);
    }
    if (json == NULL) {
      return NULL;
    }

    key  = va_arg(args, char*);
    type = va_arg(args, int);
    if (type == cJSON_Number) {
      numbervalue = va_arg(args, long);
      value       = NULL;
    } else {
      value       = va_arg(args, char*);
      numbervalue = 0;
    }
  } while (key != NULL);
  return json;
}

/**
 * @brief Generates a cJSON JSONArray from multiple strings
 * @param v1 the value for the first element.
 * @param params additional values to be added to the JSONArray
 * @note the last argument MUST be @c NULL, otherwise behavior is unspecified
 * @return a pointer to a cJSON JSONArray. Has to be freed after usage using
 * @c secFsecFreeJson
 */
cJSON* generateJSONArray(char* v1, ...) {
  initCJSON();
  syslog(LOG_AUTHPRIV | LOG_DEBUG, "Generating JSONArray");
  va_list args;
  va_start(args, v1);
  cJSON* json = cJSON_CreateArray();
  if (json == NULL) {
    oidc_seterror("Coud not create json array");
    return NULL;
  }
  char* v = v1;
  while (v != NULL) {
    // syslog(LOG_AUTHPRIV | LOG_DEBUG, "value is %s", v);
    cJSON_AddItemToArray(json, cJSON_CreateString(v));
    v = va_arg(args, char*);
  }
  return json;
}

/**
 * @brief merges two JSONObjects represented as string into one
 * @param j1 a pointer to the first json string
 * @param j2 a pointer to the second json string
 * @note for details see @c mergeJSONObjects
 * @return a pointer to a string representing the merged JSONObject. Has to be
 * freed after usage.
 */
char* mergeJSONObjectStrings(const char* j1, const char* j2) {
  if (j1 == NULL || j2 == NULL) {
    oidc_setArgNullFuncError(__func__);
    return NULL;
  }
  syslog(LOG_AUTH | LOG_DEBUG, "Merging two json objects:");
  syslog(LOG_AUTH | LOG_DEBUG, "j1 '%s'", j1);
  syslog(LOG_AUTH | LOG_DEBUG, "j2 '%s'", j2);
  initCJSON();
  cJSON* cj1 = stringToJson(j1);
  cJSON* cj2 = stringToJson(j2);
  if (cj1 == NULL || cj2 == NULL) {
    secFreeJson(cj1);
    secFreeJson(cj2);
    return NULL;
  }
  cJSON* j = mergeJSONObjects(cj1, cj2);
  secFreeJson(cj1);
  secFreeJson(cj2);
  if (j == NULL) {
    return NULL;
  }
  char* s = jsonToString(j);
  secFreeJson(j);
  syslog(LOG_AUTH | LOG_ERR, "Merge result '%s'", s);
  return s;
}

/**
 * @brief merges two CJSON JSONObjects into one.
 * The first object is used as a baseline and then elements from the second
 * object are added. This function cannot merge the objects if there are
 * conflicts. The following are no conflicts: key not existing in one object;
 * key existing in both object, but empty value for (at least) one (also for
 * empty arrays). If there are different values for the two objects for the same
 * key this is considered a merge conflict. Exception: For the key 'scope' the
 * value from @p j1 is prioritized over @p j2 without raising an error.
 * @param j1 a pointer to the first cJSON JSONObject
 * @param j2 a pointer to the second cJSON JSONObject
 * @return a pointer to a cJSON JSONObject. Has to be freed after usage using
 * @c secFreeJson
 */
cJSON* mergeJSONObjects(const cJSON* j1, const cJSON* j2) {
  if (j1 == NULL || j2 == NULL) {
    oidc_setArgNullFuncError(__func__);
    return NULL;
  }
  initCJSON();

  cJSON* json = cJSON_Duplicate(j1, cJSON_True);
  cJSON* el   = NULL;
  cJSON_ArrayForEach(el, j2) {
    char* key = el->string;
    if (jsonHasKey(j1, key)) {
      if (!cJSON_Compare(cJSON_GetObjectItemCaseSensitive(j1, key), el,
                         cJSON_True)) {
        cJSON* el1 = cJSON_GetObjectItemCaseSensitive(j1, key);
        if ((el->type == cJSON_String && !strValid(el->valuestring)) ||
            ((el->type == cJSON_Array || el->type == cJSON_Object) &&
             cJSON_GetArraySize(el) == 0)) {
          pass;
        } else if ((el1->type == cJSON_String && !strValid(el1->valuestring)) ||
                   ((el1->type == cJSON_Array || el1->type == cJSON_Object) &&
                    cJSON_GetArraySize(el1) == 0)) {
          cJSON* cpy = cJSON_Duplicate(el, cJSON_True);
          cJSON_ReplaceItemViaPointer(json, el1, cpy);
        } else if (el1->type == cJSON_String && el->type == cJSON_String &&
                   strequal(el1->valuestring, el->valuestring)) {
          pass;
        } else if (strequal("scope", key)) {
          // for scope the the value from j1 is used
          // despite the value of j2.
          // The acquired scopes might be different from the requested
          // scopes, but that's fine. Also the ordering might change
          pass;
        } else {
          oidc_errno = OIDC_EJSONMERGE;
          char* val1 = jsonToString(el1);
          char* val2 = jsonToString(el);
          syslog(LOG_AUTHPRIV | LOG_ERR,
                 "Cannot merge json objects: Conflict for key '%s' between "
                 "value '%s' and '%s'",
                 key, val1, val2);
          cJSON_free(val1);
          cJSON_free(val2);
          cJSON_Delete(json);
          return NULL;
        }
      }
    } else {
      cJSON* (*addFunc)(cJSON*, const char*, const void*) = NULL;
      void*  value                                        = NULL;
      double numbervalue                                  = 0.;
      switch (el->type) {
        case cJSON_String:
          addFunc = (cJSON * (*)(cJSON*, const char*, const void*))
              jsonAddStringValue;
          value = el->valuestring;
          break;
        case cJSON_Object:
        case cJSON_Array:
          addFunc = (cJSON * (*)(cJSON*, const char*, const void*)) jsonAddJSON;
          value   = cJSON_Duplicate(el, cJSON_True);
          break;
        case cJSON_Number: numbervalue = el->valuedouble; break;
        default:
          syslog(LOG_AUTHPRIV | LOG_ERR, "unknown type %d", el->type);
          oidc_errno = OIDC_EJSONTYPE;
          cJSON_Delete(json);
          return NULL;
      }
      if (addFunc && value) {
        json = addFunc(json, key, value);
      } else if (numbervalue) {
        json = jsonAddNumberValue(json, key, numbervalue);
      }
    }
  }
  return json;
}

/**
 * checks if an json array is empty
 * @param json the cJSON JSONArray
 * @return @c 1 if @p json is empty, @c 0 if not. If @p json is @c NULL or not
 * an array an negative error code is returned
 */
int jsonArrayIsEmpty(cJSON* json) {
  if (json == NULL) {
    return OIDC_EARGNULL;
  }
  if (json->type != cJSON_Array) {
    return OIDC_EJSONARR;
  }
  return !cJSON_GetArraySize(json);
}
