#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <syslog.h>
#include <unistd.h>

#include "../lib/jsmn.h"

#include "config.h"
#include "file_io.h"
#include "http.h"
#include "crypt.h"

#define CONFIGFILE "config.conf"
#define CRYPT_FILE "/.crypt"
#define NONCESALT_FILE "/.noncesalt"

struct oidc_provider {
  char* name;                         // mandatory in config file
  char* username;                     // optional in config file
  const char* client_id;              // mandatory in config file
  const char* client_secret;          // mandatory in config file
  const char* configuration_endpoint; // mandatory in config file
  const char* token_endpoint;         // retrieved from configuration_endpoint
  unsigned long min_valid_period;     // mandatory in config file
  char* refresh_token;                // optional in config file
  char* access_token;                 // retrieved from token endpoint
  unsigned long token_expires_at;     // retrieved from token endpoint
};

struct {
  const char* cert_path;              // mandatory in config file
  const char* wattson_url;            // optional in config file
  const char* cwd;                    // determined
  size_t cryptLen;                    // determined
  unsigned int provider_count;        // determined
  struct oidc_provider provider[];
} config;

// getter
const char* conf_getCertPath() {return config.cert_path;}
const char* conf_getWattsonUrl() {return config.wattson_url;}
const char* conf_getcwd() {return config.cwd;}
size_t conf_getCryptLen() {return config.cryptLen;}
unsigned int conf_getProviderCount() {return config.provider_count;}
char* conf_getProviderName(unsigned int provider) {return config.provider[provider].name;}
char* conf_getUsername(unsigned int provider) {return config.provider[provider].username;}
const char* conf_getClientId(unsigned int provider) {return config.provider[provider].client_id;}
const char* conf_getClientSecret(unsigned int provider) {return config.provider[provider].client_secret;}
const char* conf_getConfigurationEndpoint(unsigned int provider) {return config.provider[provider].configuration_endpoint;}
const char* conf_getTokenEndpoint(unsigned int provider) {return config.provider[provider].token_endpoint;}
unsigned long conf_getMinValidPeriod(unsigned int provider) {return config.provider[provider].min_valid_period;}
char* conf_getRefreshToken(unsigned int provider) {return config.provider[provider].refresh_token;}
char* conf_getAccessToken(unsigned int provider) {return config.provider[provider].access_token;}
unsigned long conf_getTokenExpiresAt(unsigned int provider) {return config.provider[provider].token_expires_at;}

void conf_setCryptLen(size_t size) { config.cryptLen = size; }
void conf_setTokenExpiresAt(unsigned int provider, unsigned long expires_at) {config.provider[provider].token_expires_at=expires_at;}
void conf_setUsername(unsigned int provider, char* username) {
  free(config.provider[provider].username);
  config.provider[provider].username=username;
  if(strcmp("(null)",username)==0)
    config.provider[provider].username = NULL;
}
void conf_setAccessToken(unsigned int provider, char* access_token) {
  free(config.provider[provider].access_token);
  config.provider[provider].access_token=access_token;
  if(strcmp("(null)",access_token)==0)
    config.provider[provider].access_token = NULL;
}
void conf_setRefreshToken(unsigned int provider, char* refresh_token) {
  free(config.provider[provider].refresh_token);
  config.provider[provider].refresh_token=refresh_token;
  if(strcmp("(null)",refresh_token)==0)
    config.provider[provider].refresh_token = NULL;
}

void readConfig() {
  config.cwd = getcwd(NULL,0);
  if(config.cwd==NULL) {
    syslog(LOG_AUTHPRIV|LOG_ALERT, "Could not get cwd: %m\n");
    exit(EXIT_FAILURE);
  }
  char* config_cont = readFile(CONFIGFILE);
  struct key_value pairs[3];
  pairs[0].key = "cert_path"; pairs[0].value=NULL;
  pairs[1].key = "wattson_url"; pairs[1].value=NULL;
  pairs[2].key = "provider"; pairs[2].value=NULL;
  if(getJSONValues(config_cont, pairs, sizeof(pairs)/sizeof(*pairs))<0) {
    syslog(LOG_AUTHPRIV|LOG_EMERG, "Could not parse config file. Please fix the configuration.\n");
    exit(EXIT_FAILURE);
  }
  config.cert_path = pairs[0].value;
  if (!config.cert_path || strcmp("",config.cert_path)==0) {
    syslog(LOG_AUTHPRIV|LOG_EMERG,"No cert_path found in config file '%s'.\n",CONFIGFILE);
    free(config_cont);
    exit(EXIT_FAILURE);
  }
  config.wattson_url = pairs[1].value;
  if (!config.wattson_url || strcmp("",config.wattson_url)==0) 
    syslog(LOG_AUTHPRIV|LOG_NOTICE,"No wattson_url found in config file '%s'.\n",CONFIGFILE);
  free(config_cont);
  readProviderConfig(pairs[2].value);
  free(pairs[2].value);
  // printConfig();
  logConfig();
}

char* configToJSON() {
  char* fmt = "{\n\"provider_count\":\"%u\",\n\"cert_path\":\"%s\",\n\"wattson_url\":\"%s\",\n\"cryptLen\":%lu,\n\"provider\":%s\n}";
  char* provider_fmt = "{\n%s\n}";
  char* single_provider_fmt = "\"%s\":{\n\"username\":\"%s\",\n\"client_id\":\"%s\",\n\"client_secret\":\"%s\",\n\"configuration_endpoint\":\"%s\",\n\"token_endpoint\":\"%s\",\n\"refresh_token\":\"%s\",\n\"access_token\":\"%s\",\n\"token_expires_at\":\"%lu\",\n\"min_valid_period\":\"%lu\",\n}";
  unsigned int i;
  char* provider = NULL;
  for(i=0; i<config.provider_count; i++) {
    int LEN = 2048;
    char* this_provider = NULL;
    unsigned int real_len;
    do {
      this_provider = realloc(this_provider, strlen(single_provider_fmt)+LEN+1);
      real_len = snprintf(this_provider, strlen(single_provider_fmt)+LEN+1, single_provider_fmt, conf_getProviderName(i),conf_getUsername(i),conf_getClientId(i),conf_getClientSecret(i),conf_getConfigurationEndpoint(i),conf_getTokenEndpoint(i),conf_getRefreshToken(i),conf_getAccessToken(i),conf_getTokenExpiresAt(i),conf_getMinValidPeriod(i));
      LEN*=2;
    } while(real_len>strlen(single_provider_fmt)+LEN+1);
    provider = provider ? realloc(provider, strlen(provider) + real_len+1) : calloc(sizeof(char),real_len+1);
    provider = strcat(provider, this_provider);
    free(this_provider);
  }
  char* all_provider = calloc(sizeof(char), strlen(provider_fmt)+strlen(provider)+1);
  sprintf(all_provider, provider_fmt, provider);
  free(provider);
  char* json = NULL;
  unsigned int real_len;
  int LEN=2048;
  do {
    json = json ? realloc(json, strlen(fmt)+LEN+1) : calloc(sizeof(char), strlen(fmt)+LEN+1);
    real_len = snprintf(json, strlen(fmt)+LEN+1, fmt,conf_getProviderCount(),conf_getCertPath(),conf_getWattsonUrl(),conf_getCryptLen(), all_provider);
    LEN*=2;
  } while(real_len>strlen(fmt)+LEN+1);
  free(all_provider);
  return json;
}

void printConfig() {
  char* jsonConfig = configToJSON();
  fprintf(stdout, "%s\n", jsonConfig);
  free(jsonConfig);
}

void logConfig() {
  syslog(LOG_AUTHPRIV|LOG_DEBUG,"Number of providers: %d\n", config.provider_count);
  syslog(LOG_AUTHPRIV|LOG_DEBUG,"cert_path: %s\n", config.cert_path);
  syslog(LOG_AUTHPRIV|LOG_DEBUG,"wattson_url: %s\n", config.wattson_url);
  unsigned int i;
  for (i=0; i<config.provider_count; i++) {
    syslog(LOG_AUTHPRIV|LOG_DEBUG,"Provider %d %s:\n",i,config.provider[i].name);
    syslog(LOG_AUTHPRIV|LOG_DEBUG,"username: %s, client_id: %s, client_secret: %s, configuration_endpoint: %s, token_endpoint: %s, refresh_token: %s, access_token: %s, token_expires_at: %lu, minimum period of validity: %lu\n",
        config.provider[i].username, config.provider[i].client_id, config.provider[i].client_secret, config.provider[i].configuration_endpoint, config.provider[i].token_endpoint, config.provider[i].refresh_token, config.provider[i].access_token, config.provider[i].token_expires_at, config.provider[i].min_valid_period);
  }
}

void writeEncryptedConfig() {
  char* jsonconfig = configToJSON();
  unsigned char* encrypted = encrypt((unsigned char*) jsonconfig, "password");
  free(jsonconfig);
  writeBufferToFile(CRYPT_FILE, (const char*) encrypted, MAC_LEN +conf_getCryptLen());
  free(encrypted);
  writeNonceSalt(NONCESALT_FILE); 
}

void readEncryptedConfig() {
  readNonceSalt(NONCESALT_FILE);
  char* encrypted = readFile(CRYPT_FILE);
  if(NULL==encrypted) {
    syslog(LOG_AUTHPRIV|LOG_NOTICE, "could not read encrypted config from file '%s'\n",CRYPT_FILE);
    return;
  }
  unsigned char* decrypted = decrypt((unsigned char*)encrypted, "password");

  struct key_value pairs[2];
  pairs[0].key = "provider_count"; pairs[0].value = NULL;
  pairs[1].key = "provider"; pairs[1].value = NULL;
  if(getJSONValues((char*)decrypted, pairs, sizeof(pairs)/sizeof(*pairs))<0) {
    syslog(LOG_AUTHPRIV|LOG_ALERT, "Could not parse the encrypted config.\n");
    free(decrypted);
    return;
  }
  free(decrypted);
  if(NULL==pairs[0].value) {
    syslog(LOG_AUTHPRIV|LOG_ALERT, "Could not get provider_count from the parsed encrypted config.\n");
    free(pairs[1].value);
    return;
  }
  config.provider_count = atoi(pairs[0].value);
  free(pairs[0].value);


  int r;
  jsmn_parser p;
  jsmn_init(&p);
  int token_needed = jsmn_parse(&p,pairs[1].value, strlen(pairs[1].value), NULL, 0);
  jsmntok_t t[token_needed]; 
  jsmn_init(&p);
  r = jsmn_parse(&p,pairs[1].value, strlen(pairs[1].value), t, sizeof(t)/sizeof(t[0]));

  if (!checkParseResult(r, t[0]))
    return;
  int t_index = 2;
  unsigned int i;
  for(i=0;i<config.provider_count;i++){
    conf_setUsername(i, getValuefromTokens(&t[t_index], t[t_index].size*2, "username",pairs[1].value));
    conf_setRefreshToken(i, getValuefromTokens(&t[t_index], t[t_index].size*2, "refresh_token",pairs[1].value)); 
    conf_setAccessToken(i, getValuefromTokens(&t[t_index], t[t_index].size*2, "access_token",pairs[1].value)); 
    char* exp_at = getValuefromTokens(&t[t_index], t[t_index].size*2, "token_expires_at",pairs[1].value);
    if(NULL!=exp_at) 
      conf_setTokenExpiresAt(i, atoi(exp_at));
    free(exp_at);
    t_index += t[t_index].size*2+2;
  }
  free(pairs[1].value);
}

void readProviderConfig(char* provider) {
  int r;
  jsmn_parser p;
  jsmn_init(&p);
  int token_needed = jsmn_parse(&p, provider, strlen(provider), NULL, 0);
  jsmntok_t t[token_needed]; 
  jsmn_init(&p);
  r = jsmn_parse(&p, provider, strlen(provider), t, sizeof(t)/sizeof(t[0]));

  if (!checkParseResult(r, t[0]))
    exit(EXIT_FAILURE);
  config.provider_count = t[0].size;
  int t_index = 2;
  unsigned int i;
  for(i=0;i<config.provider_count;i++){
    config.provider[i].name = malloc(t[t_index-1].end-t[t_index-1].start+1);
    sprintf(config.provider[i].name,"%.*s", t[t_index-1].end-t[t_index-1].start,
        provider + t[t_index-1].start);
    conf_setUsername(i, getValuefromTokens(&t[t_index], t[t_index].size*2, "username", provider));
    config.provider[i].client_id = getValuefromTokens(&t[t_index], t[t_index].size*2, "client_id", provider);
    config.provider[i].client_secret = getValuefromTokens(&t[t_index], t[t_index].size*2, "client_secret", provider);
    config.provider[i].configuration_endpoint = getValuefromTokens(&t[t_index], t[t_index].size*2, "configuration_endpoint", provider);
    char* refresh_token = getValuefromTokens(&t[t_index], t[t_index].size*2, "refresh_token", provider);
    if (refresh_token!=NULL  && strcmp("", refresh_token)!=0)
      conf_setRefreshToken(i, refresh_token); 
    char* pov = getValuefromTokens(&t[t_index], t[t_index].size*2, "min_valid_period", provider);
    if(NULL==config.provider[i].client_id) {
      syslog(LOG_AUTHPRIV|LOG_EMERG, "No client_id found for provider %s in config file '%s'.\n",config.provider[i].name, CONFIGFILE);
      exit(EXIT_FAILURE);
    }
    if(NULL==config.provider[i].client_secret) {
      syslog(LOG_AUTHPRIV|LOG_EMERG, "No client_secret found for provider %s in config file '%s'.\n",config.provider[i].name, CONFIGFILE);
      exit(EXIT_FAILURE);
    }
    if(NULL==config.provider[i].configuration_endpoint) {
      syslog(LOG_AUTHPRIV|LOG_EMERG, "No configuration_endpoint found for provider %s in config file '%s'.\n",config.provider[i].name, CONFIGFILE);
      exit(EXIT_FAILURE);
    }
    if(NULL==pov) {
      syslog(LOG_AUTHPRIV|LOG_EMERG, "No min_valid_period found for provider %s in config file '%s'.\n",config.provider[i].name, CONFIGFILE);
      exit(EXIT_FAILURE);
    }
    config.provider[i].min_valid_period = atoi(pov);
    free(pov);
    if (NULL==config.provider[i].refresh_token||strcmp("",config.provider[i].refresh_token)==0) {
      syslog(LOG_AUTHPRIV|LOG_NOTICE, "No refresh_token found for provider %s in config file '%s'.\n",config.provider[i].name, CONFIGFILE);
    }
    if (NULL==config.provider[i].username||strcmp("",config.provider[i].username)==0) {
      // config.provider[i].username = "";
      syslog(LOG_AUTHPRIV|LOG_NOTICE, "No username found for provider %s in config file '%s'.\n",config.provider[i].name, CONFIGFILE);
    }
    t_index += t[t_index].size*2+2;
    getOIDCProviderConfig(i);
  }

}

void getOIDCProviderConfig(int index) {
  char* res = httpsGET(config.provider[index].configuration_endpoint);
  config.provider[index].token_endpoint = getJSONValue(res, "token_endpoint");
  free(res);
  if(NULL==config.provider[index].token_endpoint) {
    syslog(LOG_AUTHPRIV|LOG_EMERG, "Could not get token_endpoint from the configuration_endpoint. Please check the configuration_endpoint URL for provider %s in config file '%s'.\n",config.provider[index].name, CONFIGFILE);
    exit(EXIT_FAILURE);
  }
}

int jsoneq(const char *json, jsmntok_t *tok, const char *s) {
  if (tok->type == JSMN_STRING && (int) strlen(s) == tok->end - tok->start &&
      strncmp(json + tok->start, s, tok->end - tok->start) == 0) {
    return 0;
  }
  return -1;
}

int checkParseResult(int r, jsmntok_t t) {
  if (r < 0) {
    syslog(LOG_AUTHPRIV|LOG_ERR, "Failed to parse JSON: %d\n", r);
    return 0;
  }

  /* Assume the top-level element is an object */
  if (r < 1 || t.type != JSMN_OBJECT) {
    syslog(LOG_AUTHPRIV|LOG_ERR, "Object expected\n");
    return 0;
  }
  return 1;
}

int getJSONValues(const char* json, struct key_value* pairs, size_t size) {
  int r;
  jsmn_parser p;
  jsmn_init(&p);
  int token_needed = jsmn_parse(&p, json, strlen(json), NULL, 0);
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "Token needed for parsing: %d",token_needed);
  jsmntok_t t[token_needed]; 
  jsmn_init(&p);
  r = jsmn_parse(&p, json, strlen(json), t, sizeof(t)/sizeof(t[0]));

  if(!checkParseResult(r, t[0]))
    return -1;
  unsigned int i;
  for(i=0; i<size;i++){
    pairs[i].value = getValuefromTokens(t, r, pairs[i].key, json);
  }
  return i;
}

char* getJSONValue(const char* json, const char* key) {
  int r;
  jsmn_parser p;
  jsmn_init(&p);
  int token_needed = jsmn_parse(&p, json, strlen(json), NULL, 0);
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "Token needed for parsing: %d",token_needed);
  jsmntok_t t[token_needed]; 
  jsmn_init(&p);
  r = jsmn_parse(&p, json, strlen(json), t, sizeof(t)/sizeof(t[0]));

  if(checkParseResult(r, t[0]))	
    return getValuefromTokens(t, r, key, json);
  else
    return NULL;
}

char* getValuefromTokens(jsmntok_t t[], int r, const char* key, const char* json) {
  /* Loop over all keys of the root object */
  int i;
  for (i = 1; i < r; i++) {
    if (jsoneq(json, &t[i], key) == 0) {
      /* We may use strndup() to fetch string value */
      char* value = calloc(sizeof(char),t[i+1].end-t[i+1].start+1);
      sprintf(value,"%.*s", t[i+1].end-t[i+1].start,
          json + t[i+1].start);
      return value;
    } 
  }
  return NULL;
}
