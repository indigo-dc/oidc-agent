#include "json.h"

#include "oidc_error.h"
#include "utils/listUtils.h"
#include "utils/stringUtils.h"

#include <stdarg.h>
#include <syslog.h>

cJSON_Hooks hooks;
int         jsonInitDone = 0;

void initCJSON() {  // TODO
  if (!jsonInitDone) {
    hooks.malloc_fn = secAlloc;
    hooks.free_fn   = secFree;
    cJSON_InitHooks(&hooks);
    jsonInitDone = 1;
  }
}
char* jsonToString(cJSON* cjson) {
  if (cjson == NULL) {
    oidc_setArgNullFuncError(__func__);
    return NULL;
  }
  return cJSON_Print(cjson);
}

cJSON* stringToJson(const char* json) {
  if (NULL == json) {
    oidc_setArgNullFuncError(__func__);
    return NULL;
  }
  syslog(LOG_AUTHPRIV | LOG_DEBUG, "Parsing json '%s'", json);
  cJSON* cj = cJSON_Parse(json);
  if (cj == NULL) {
    oidc_errno = OIDC_EJSONPARS;
  }
  return cj;
}

int isJSONObject(const char* json) {
  if (NULL == json) {
    oidc_setArgNullFuncError(__func__);
    return 0;
  }
  cJSON* cj = stringToJson(json);
  if (cj == NULL) {
    return 0;
  }
  return cJSON_IsObject(cj);
}

void secFreeJson(cJSON* cjson) {
  if (cjson == NULL) {
    return;
  }
  cJSON_Delete(cjson);
}

int jsonStringHasKey(const char* json, const char* key) {
  if (NULL == json || NULL == key) {
    oidc_setArgNullFuncError(__func__);
    return 0;
  }
  cJSON* cj = stringToJson(json);
  if (cj == NULL) {
    return 0;
  }
  int res = jsonHasKey(cj, key);
  secFreeJson(cj);
  return res;
}

int jsonHasKey(const cJSON* cjson, const char* key) {
  if (NULL == cjson || NULL == key) {
    oidc_setArgNullFuncError(__func__);
    return 0;
  }
  char* value = getJSONValue(cjson, key);
  if (strValid(value)) {
    secFree(value);
    return 1;
  } else {
    return 0;
  }
}

char* getJSONItemValue(cJSON* valueItem) {
  if (NULL == valueItem) {
    oidc_setArgNullFuncError(__func__);
    return NULL;
  }
  if (cJSON_IsString(valueItem)) {
    return oidc_strcopy(cJSON_GetStringValue(valueItem));
  }
  return cJSON_Print(valueItem);
}

char* getJSONValue(const cJSON* cjson, const char* key) {
  if (NULL == cjson || NULL == key) {
    oidc_setArgNullFuncError(__func__);
    return NULL;
  }
  if (!cJSON_HasObjectItem(cjson, key)) {
    oidc_errno = OIDC_EJSONNOFOUND;
    return NULL;
  }
  cJSON* valueItem = cJSON_GetObjectItemCaseSensitive(cjson, key);
  return getJSONItemValue(valueItem);
}

char* getJSONValueFromString(const char* json, const char* key) {
  if (NULL == json || NULL == key) {
    oidc_setArgNullFuncError(__func__);
    return NULL;
  }
  cJSON* cj = stringToJson(json);
  if (cj == NULL) {
    return NULL;
  }
  char* value = getJSONValue(cj, key);
  secFreeJson(cj);
  return value;
}

oidc_error_t getJSONValues(const cJSON* cjson, struct key_value* pairs,
                           size_t size) {
  if (NULL == cjson || NULL == pairs || size == 0) {
    oidc_setArgNullFuncError(__func__);
    return oidc_errno;
  }
  unsigned int i;
  for (i = 0; i < size; i++) {
    pairs[i].value = getJSONValue(cjson, pairs[i].key);
  }
  return i;
}

oidc_error_t getJSONValuesFromString(const char* json, struct key_value* pairs,
                                     size_t size) {
  if (NULL == json || NULL == pairs || size == 0) {
    oidc_setArgNullFuncError(__func__);
    return oidc_errno;
  }
  cJSON* cj = stringToJson(json);
  if (cj == NULL) {
    return oidc_errno;
  }
  oidc_error_t e = getJSONValues(cj, pairs, size);
  secFreeJson(cj);
  return e;
}

list_t* JSONArrayToList(const cJSON* cjson) {
  if (NULL == cjson) {
    oidc_setArgNullFuncError(__func__);
    return NULL;
  }
  if (!cJSON_IsArray(cjson)) {
    oidc_errno = OIDC_EJSONARR;
    return NULL;
  }

  int     j;
  list_t* l = list_new();
  l->free   = secFree;
  l->match  = (int (*)(void*, void*)) & strequal;
  for (j = 0; j < cJSON_GetArraySize(cjson); j++) {
    list_rpush(l, list_node_new(oidc_strcopy(
                      getJSONItemValue(cJSON_GetArrayItem(cjson, j)))));
  }
  return l;
}

list_t* JSONArrayStringToList(const char* json) {
  if (NULL == json) {
    oidc_setArgNullFuncError(__func__);
    return NULL;
  }
  cJSON* cj = stringToJson(json);
  if (cj == NULL) {
    return NULL;
  }
  list_t* l = JSONArrayToList(cj);
  secFreeJson(cj);
  return l;
}

char* JSONArrayToDelimitedString(const cJSON* cjson, char delim) {
  if (NULL == cjson) {
    oidc_setArgNullFuncError(__func__);
    return NULL;
  }
  list_t* list = JSONArrayToList(cjson);
  char*   str  = listToDelimitedString(list, delim);
  list_destroy(list);
  return str;
}

char* JSONArrayStringToDelimitedString(const char* json, char delim) {
  if (NULL == json) {
    oidc_setArgNullFuncError(__func__);
    return NULL;
  }
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
  cJSON_AddStringToObject(cjson, key, value);
  return cjson;
}
cJSON* jsonAddNumberValue(cJSON* cjson, const char* key, const double value) {
  if (NULL == cjson || NULL == key) {
    oidc_setArgNullFuncError(__func__);
    return NULL;
  }
  cJSON_AddNumberToObject(cjson, key, value);
  return cjson;
}
cJSON* jsonAddArrayValue(cJSON* cjson, const char* key,
                         const char* json_array) {
  if (NULL == cjson || NULL == key || NULL == json_array) {
    oidc_setArgNullFuncError(__func__);
    return NULL;
  }
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
  cJSON* object = stringToJson(json_object);
  cJSON_AddItemToObject(cjson, key, object);
  return cjson;
}
cJSON* jsonAddJSON(cJSON* cjson, const char* key, cJSON* item) {
  if (NULL == cjson || NULL == key || NULL == item) {
    oidc_setArgNullFuncError(__func__);
    return NULL;
  }
  cJSON_AddItemToObject(cjson, key, item);
  return cjson;
}

cJSON* listToJSONArray(list_t* list) {
  if (list == NULL) {
    oidc_setArgNullFuncError(__func__);
    return NULL;
  }
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
 * last argument has to be NULL
 * Only use pairs of 3 (char*, char*, int)
 */
cJSON* generateJSONObject(char* k1, char* v1, int type1, ...) {
  syslog(LOG_AUTHPRIV | LOG_DEBUG, "Generating JSONObject");
  va_list args;
  va_start(args, type1);
  syslog(LOG_AUTHPRIV | LOG_DEBUG, "First key:value is %s:%s", k1, v1);
  cJSON* json = cJSON_CreateObject();
  if (json == NULL) {
    oidc_seterror("Coud not create json object");
    return NULL;
  }
  char* key   = k1;
  char* value = v1;
  int   type  = type1;
  do {
    syslog(LOG_AUTHPRIV | LOG_DEBUG, "key:value is %s:%s", key, value);
    cJSON* (*addFunc)(cJSON*, const char*, const char*);
    switch (type) {
      case cJSON_String: addFunc = jsonAddStringValue; break;
      case cJSON_Object: addFunc = jsonAddObjectValue; break;
      case cJSON_Array: addFunc = jsonAddArrayValue; break;
      case cJSON_Number:
        syslog(LOG_AUTHPRIV | LOG_ERR,
               "generating JSONObjects with numbers not supported");
        oidc_errno = OIDC_NOTIMPL;
        return NULL;
      default:
        syslog(LOG_AUTHPRIV | LOG_ERR, "unknown type %d", type);
        oidc_errno = OIDC_EJSONTYPE;
        return NULL;
    }
    json = addFunc(json, key, value);
    if (json == NULL) {
      return NULL;
    }

    key   = va_arg(args, char*);
    value = va_arg(args, char*);
    type  = va_arg(args, int);
  } while (key != NULL);
  return json;
}

/**
 * last argument has to be NULL
 */
cJSON* generateJSONArray(char* v1, ...) {
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
    syslog(LOG_AUTHPRIV | LOG_DEBUG, "value is %s", v);
    cJSON_AddItemToArray(json, cJSON_CreateString(v));
    v = va_arg(args, char*);
  }
  return json;
}

char* mergeJSONObjectStrings(const char* j1, const char* j2) {
  if (j1 == NULL || j2 == NULL) {
    oidc_setArgNullFuncError(__func__);
    return NULL;
  }
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
  return s;
}

cJSON* mergeJSONObjects(const cJSON* j1, const cJSON* j2) {
  if (j1 == NULL || j2 == NULL) {
    oidc_setArgNullFuncError(__func__);
    return NULL;
  }

  cJSON* json = cJSON_Duplicate(j1, cJSON_True);
  cJSON* el   = NULL;
  cJSON_ArrayForEach(el, j2) {
    char* key = el->string;
    if (jsonHasKey(j1, key)) {
      // TODO compare
      if (!cJSON_Compare(cJSON_GetObjectItemCaseSensitive(j1, key), el,
                         cJSON_True)) {
        oidc_errno = OIDC_EJSONMERGE;
        char* val1 = jsonToString(cJSON_GetObjectItemCaseSensitive(j1, key));
        char* val2 = jsonToString(el);
        syslog(LOG_AUTHPRIV | LOG_ERR,
               "Cannot merge json objects: Conflict for key '%s' between "
               "value '%s' and '%s'",
               key, val1, val2);
        cJSON_free(val1);
        cJSON_free(val2);
        cJSON_Delete(json);
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
          value   = el->child;
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
