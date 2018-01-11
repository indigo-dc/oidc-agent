#ifndef OIDC_JSON_H
#define OIDC_JSON_H

#include "../lib/jsmn/jsmn.h"

#include "oidc_error.h"

struct key_value {
  const char* key;
  char* value;
};


char* getJSONValue(const char* json, const char* key) ;
int getJSONValues(const char* json, struct key_value* pairs, size_t size) ;
char* getValuefromTokens(jsmntok_t t[], int r, const char* key, const char* json) ;
int jsoneq(const char *json, jsmntok_t *tok, const char *s) ;
int checkParseResult(int r, jsmntok_t t) ;
char* json_addValue(char* json, const char* key, const char* value) ;
char* json_addStringValue(char* json, const char* key, char* value) ;
char* json_arrAdd(char* json, const char* value) ;
int json_hasKey(char* json, const char* key) ;
oidc_error_t checkArrayParseResult(int r, jsmntok_t t) ;
int JSONArrrayToArray(const char* json, char** arr) ;

#endif // OIDC_JSON_H
