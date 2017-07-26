#ifndef CONFIG_H
#define CONFIG_H

#include "../lib/jsmn.h"



struct key_value {
  const char* key;
  char* value;
};

void readConfig();
void readEncryptedConfig() ;
void writeEncryptedConfig() ;
void printConfig() ;
void logConfig() ;
char* configToJSON();

const char* conf_getCertPath();
const char* conf_getWattsonUrl();
const char* conf_getcwd();
// unsigned char* conf_getSalt();
// unsigned char* conf_getNonce();
size_t conf_getCryptLen();
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
unsigned long conf_getTokenExpiresAt(unsigned int provider);

// void conf_setSalt(unsigned char* salt);
// void conf_setNonce(unsigned char* nonce);
void conf_setCryptLen(size_t size);
void conf_setUsername(unsigned int provider, char* username);
void conf_setAccessToken(unsigned int provider, char* access_token);
void conf_setRefreshToken(unsigned int provider, char* refresh_token);
void conf_setTokenExpiresAt(unsigned int provider, unsigned long expires_at);

void readProviderConfig(char* provider) ;
void getOIDCProviderConfig(int i) ;
int jsoneq(const char *json, jsmntok_t *tok, const char *s) ;
int checkParseResult(int r, jsmntok_t t) ;
char* getJSONValue(const char* json, const char* key) ;
int getJSONValues(const char* json, struct key_value* pairs, size_t size) ;
char* getValuefromTokens(jsmntok_t t[], int r, const char* key, const char* json) ;

#endif //CONFIG_H
