#ifndef CONFIG_H
#define CONFIG_H

#include "../lib/jsmn.h"



struct key_value {
  const char* key;
  char* value;
};

void readConfig();
void printConfig() ;
void logConfig() ;

const char* conf_getCertPath();
const char* conf_getWattsonUrl();
const char* conf_getcwd();
unsigned int conf_getProviderCount();
char* conf_getProviderName(unsigned int provider);
char* conf_getUsername(unsigned int provider);
const char* conf_getClientId(unsigned int provider);
const char* conf_getClientSecret(unsigned int provider);
const char* conf_getConfigurationEndpoint(unsigned int provider);
const char* conf_getTokenEndpoint(unsigned int provider);
unsigned long conf_getMinValidPeriod(unsigned int provider);
char* conf_getRefreshToken(unsigned int provider);
char* conf_getAccessToken(unsigned int provider);
unsigned long conf_getTokenExpiresIn(unsigned int provider);

void conf_setUsername(unsigned int provider, char* username);
void conf_setAccessToken(unsigned int provider, char* access_token);
void conf_setRefreshToken(unsigned int provider, char* refresh_token);
void conf_setTokenExpiresIn(unsigned int provider, unsigned long expires_in);

void readProviderConfig(char* provider) ;
void getOIDCProviderConfig(int i) ;
int jsoneq(const char *json, jsmntok_t *tok, const char *s) ;
int checkParseResult(int r, jsmntok_t t) ;
char* getJSONValue(const char* json, const char* key) ;
int getJSONValues(const char* json, struct key_value* pairs, size_t size) ;
char* getValuefromTokens(jsmntok_t t[], int r, const char* key, const char* json) ;

#endif //CONFIG_H
