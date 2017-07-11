#ifndef CONFIG_H
#define CONFIG_H

#include "../lib/jsmn.h"

struct {
  const char* client_id;
  const char* client_secret;
  const char* configuration_endpoint;
  const char* refresh_token;
  const char* token_endpoint;
  const char* cert_path;
} config;

void readConfig(const char* filename);
void getOIDCProviderConfig() ;
static int jsoneq(const char *json, jsmntok_t *tok, const char *s) ;

#endif //CONFIG_H
