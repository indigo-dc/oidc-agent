#ifndef OIDC_JSON_H
#define OIDC_JSON_H

#include "key_value.h"
#include "oidc_error.h"

#include "../lib/jsmn/jsmn.h"
#include "../lib/list/src/list.h"

char* getJSONValue(const char* json, const char* key);
int   getJSONValues(const char* json, struct key_value* pairs, size_t size);
char* getValuefromTokens(jsmntok_t t[], int r, const char* key,
                         const char* json);
int   jsoneq(const char* json, jsmntok_t* tok, const char* s);
int   checkParseResult(int r, jsmntok_t t);
char* json_addValue(char* json, const char* key, const char* value);
char* json_addStringValue(char* json, const char* key, char* value);
char* json_arrAdd(char* json, const char* value);
int   json_hasKey(char* json, const char* key);
oidc_error_t checkArrayParseResult(int r, jsmntok_t t);
int          JSONArrrayToArray(const char* json, char** arr);
char*        JSONArrrayToDelimitedString(const char* json, char delim);
list_t*      JSONArrayToList(const char* json);
int          isJSONObject(const char* json);
char*        generateJSONObject(char* k1, char* v1, int isString1, ...);
char*        generateJSONArray(char* v1, ...);

#endif  // OIDC_JSON_H
