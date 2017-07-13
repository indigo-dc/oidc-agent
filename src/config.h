#ifndef CONFIG_H
#define CONFIG_H

#include "../lib/jsmn.h"


struct oidc_provider {
  const char* username;
  const char* client_id;
  const char* client_secret;
  const char* configuration_endpoint;
  const char* token_endpoint;
  unsigned long expiration_duration;
  char* refresh_token;
  char* access_token;
  unsigned long token_expires_in;
};

struct {
  const char* cert_path;
  unsigned int provider_count;
  struct oidc_provider provider[];
} config;

struct key_value {
  const char* key;
  char* value;
};

void readConfig();
void printConfig() ;
char* readFile(const char* filename) ;
void readProviderConfig() ;
void getOIDCProviderConfig() ;
int jsoneq(const char *json, jsmntok_t *tok, const char *s) ;
int checkParseResult(int r, jsmntok_t t) ;
char* getJSONValue(const char* json, const char* key) ;
int getJSONValues(const char* json, struct key_value* pairs, size_t size) ;
char* getValuefromTokens(jsmntok_t t[], int r, const char* key, const char* json) ;

#endif //CONFIG_H
