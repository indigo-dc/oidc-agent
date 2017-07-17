#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <syslog.h>

#include "../lib/jsmn.h"

#include "config.h"
#include "file_io.h"
#include "http.h"

#define CONFIGFILE "config.conf"

struct oidc_provider {
  char* name;
  char* username;
  const char* client_id;
  const char* client_secret;
  const char* configuration_endpoint;
  const char* token_endpoint;
  unsigned long min_valid_period;
  char* refresh_token;
  char* access_token;
  unsigned long token_expires_in;
};

struct {
  const char* cert_path;
  unsigned int provider_count;
  struct oidc_provider provider[];
} config;

// getter
const char* conf_getCertPath() {return config.cert_path;}
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
unsigned long conf_getTokenExpiresIn(unsigned int provider) {return config.provider[provider].token_expires_in;}

// setter
void conf_setTokenExpiresIn(unsigned int provider, unsigned long expires_in) {config.provider[provider].token_expires_in=expires_in;}
void conf_setUsername(unsigned int provider, char* username) {
  free(config.provider[provider].username);
  config.provider[provider].username=username;
}
void conf_setAccessToken(unsigned int provider, char* access_token) {
  free(config.provider[provider].access_token);
  config.provider[provider].access_token=access_token;
}
void conf_setRefreshToken(unsigned int provider, char* refresh_token) {
  free(config.provider[provider].refresh_token);
  config.provider[provider].refresh_token=refresh_token;
}

void readConfig() {
  char* config_cont = readFile(CONFIGFILE);
  config.cert_path = getJSONValue(config_cont, "cert_path");
  if (!config.cert_path || strcmp("",config.cert_path)==0) {
    syslog(LOG_AUTHPRIV|LOG_EMERG,"No cert_path found in config file '%s'.\n",CONFIGFILE);
    free(config_cont);
    exit(EXIT_FAILURE);
  }
  char* provider = getJSONValue(config_cont, "provider");
  free(config_cont);
  readProviderConfig(provider);
  free(provider);
  printConfig();
}

void printConfig() {
  fprintf(stdout,"Number of providers: %d\n", config.provider_count);
  fprintf(stdout,"cert_path: %s\n", config.cert_path);
  unsigned int i;
  for (i=0; i<config.provider_count; i++) {
    fprintf(stdout,"Provider %d %s:\n",i,config.provider[i].name);
    fprintf(stdout,"\tusername: %s\n\tclient_id: %s\n\tclient_secret: %s\n\tconfiguration_endpoint: %s\n\ttoken_endpoint: %s\n\trefresh_token: %s\n\taccess_token: %s\n\ttoken_expires_in: %lu\n\tminimum period of validity: %lu\n",
        config.provider[i].username, config.provider[i].client_id, config.provider[i].client_secret, config.provider[i].configuration_endpoint, config.provider[i].token_endpoint, config.provider[i].refresh_token, config.provider[i].access_token, config.provider[i].token_expires_in, config.provider[i].min_valid_period);
  }
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
    config.provider[i].username = getValuefromTokens(&t[t_index], t[t_index].size*2, "username", provider);
    config.provider[i].client_id = getValuefromTokens(&t[t_index], t[t_index].size*2, "client_id", provider);
    config.provider[i].client_secret = getValuefromTokens(&t[t_index], t[t_index].size*2, "client_secret", provider);
    config.provider[i].configuration_endpoint = getValuefromTokens(&t[t_index], t[t_index].size*2, "configuration_endpoint", provider);
    config.provider[i].refresh_token = getValuefromTokens(&t[t_index], t[t_index].size*2, "refresh_token", provider);
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
    if (NULL==config.provider[i].refresh_token) {
      config.provider[i].refresh_token = "";
      syslog(LOG_AUTHPRIV|LOG_NOTICE, "No refresh_token found for provider %s in config file '%s'.\n",config.provider[i].name, CONFIGFILE);
    }
    if (NULL==config.provider[i].username) {
      config.provider[i].username = "";
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
      char* value = malloc(t[i+1].end-t[i+1].start+1);
      sprintf(value,"%.*s", t[i+1].end-t[i+1].start,
          json + t[i+1].start);
      return value;
    } 
  }
  return NULL;
}
