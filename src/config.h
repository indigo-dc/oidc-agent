#ifndef CONFIG_H
#define CONFIG_H

#include "../lib/jsmn.h"


struct oidc_provider {
  const char* client_id;
  const char* client_secret;
  const char* refresh_token;
  const char* configuration_endpoint;
  const char* token_endpoint;
};

struct {
  const char* cert_path;
  struct oidc_provider provider[];
} config;

void readConfig();
void printConfig() ;
char* readFile(const char* filename) ;
void readProviderConfig() ;
void getOIDCProviderConfig() ;
static int jsoneq(const char *json, jsmntok_t *tok, const char *s) ;
int checkParseResult(int r, jsmntok_t t) ;
char* getJSONValue(const char* json, const char* key) ;
char* getValuefromTokens(jsmntok_t t[], int r, const char* key, const char* json) ;

#endif //CONFIG_H
