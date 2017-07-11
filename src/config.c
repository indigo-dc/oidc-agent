#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../lib/jsmn.h"

#include "config.h"
#include "http.h"

#define CONFIGFILE "config.conf"

void readConfig() {
  char* config_cont = readFile(CONFIGFILE);
  char* provider = getJSONValue(config_cont, "provider");
  config.cert_path = getJSONValue(config_cont, "cert_path");
  if (!config.cert_path || strcmp("",config.cert_path)==0) {
    fprintf(stderr,"No cert_path found in config file.");
  }
  free(config_cont);
  readProviderConfig(provider);
  config.provider_count = getJSONChildNumber(provider);
  printConfig(config.provider_count);
  free(provider);
}

void printConfig(int providerCount) {
  printf("Provider: %d\n", providerCount);
  printf("cert_path: %s\n", config.cert_path);
  int i;
  for (i=0; i<providerCount; i++) {
    printf("username: %s\nclient_id: %s\nclient_secret: %s\nconfiguration_endpoint: %s\ntoken_endpoint: %s\nrefresh_token: %s\n",
        config.provider[i].username, config.provider[i].client_id, config.provider[i].client_secret, config.provider[i].configuration_endpoint, config.provider[i].token_endpoint, config.provider[i].refresh_token);
  }
}

char* readFile(const char* filename) {
  printf("Reading file: %s\n", filename);
  FILE *fp;
  long lSize;
  char *buffer;

  fp = fopen ( filename, "rb" );
  if( !fp ) perror(filename),exit(EXIT_FAILURE);

  fseek( fp , 0L , SEEK_END);
  lSize = ftell( fp );
  rewind( fp );

  buffer = calloc( 1, lSize+1 );
  if( !buffer ) fclose(fp),fputs("memory alloc fails",stderr),exit(1);

  if( 1!=fread( buffer , lSize, 1 , fp) )
    fclose(fp),free(buffer),fputs("entire read fails",stderr),exit(1);

  fclose(fp);
  return buffer;
}

void readProviderConfig(char* provider) {

  int r;
  jsmn_parser p;
  jsmn_init(&p);
  int token_needed = jsmn_parse(&p, provider, strlen(provider), NULL, 0);
  //printf("Token needed: %d\n",token_needed);
  jsmntok_t t[token_needed]; 
  jsmn_init(&p);
  r = jsmn_parse(&p, provider, strlen(provider), t, sizeof(t)/sizeof(t[0]));

  if(!checkParseResult(r, t[0]))
    exit(EXIT_FAILURE);
  int t_index = 2;
  int i;
  for(i=0;i<getJSONChildNumber(provider);i++){
      config.provider[i].username = getValuefromTokens(&t[t_index], t[t_index].size*2, "username", provider);
      config.provider[i].client_id = getValuefromTokens(&t[t_index], t[t_index].size*2, "client_id", provider);
      config.provider[i].client_secret = getValuefromTokens(&t[t_index], t[t_index].size*2, "client_secret", provider);
      config.provider[i].configuration_endpoint = getValuefromTokens(&t[t_index], t[t_index].size*2, "configuration_endpoint", provider);
      config.provider[i].refresh_token = getValuefromTokens(&t[t_index], t[t_index].size*2, "refresh_token", provider);
      if (!config.provider[i].refresh_token)
        config.provider[i].refresh_token = "";
      t_index += t[t_index].size*2+2;
      getOIDCProviderConfig(i);
  }

}

void getOIDCProviderConfig(int index) {
  const char* res = httpsGET(config.provider[index].configuration_endpoint);
  config.provider[index].token_endpoint = getJSONValue(res, "token_endpoint");
}

static int jsoneq(const char *json, jsmntok_t *tok, const char *s) {
  if (tok->type == JSMN_STRING && (int) strlen(s) == tok->end - tok->start &&
      strncmp(json + tok->start, s, tok->end - tok->start) == 0) {
    return 0;

  }
  return -1;

}

int checkParseResult(int r, jsmntok_t t) {
  if (r < 0) {
    fprintf(stderr, "Failed to parse JSON: %d\n", r);
    return 0;
  }

  /* Assume the top-level element is an object */
  if (r < 1 || t.type != JSMN_OBJECT) {
    fprintf(stderr, "Object expected\n");
    return 0;
  }
  return 1;

}

char* getJSONValue(const char* json, const char* key) {
  int r;
  jsmn_parser p;
  jsmn_init(&p);
  int token_needed = jsmn_parse(&p, json, strlen(json), NULL, 0);
  //printf("Token needed: %d\n",token_needed);
  jsmntok_t t[token_needed]; 
  jsmn_init(&p);
  r = jsmn_parse(&p, json, strlen(json), t, sizeof(t)/sizeof(t[0]));

  if(checkParseResult(r, t[0]))	
    return getValuefromTokens(t, r, key, json);
  else
    return NULL;
}

int getJSONChildNumber(const char* json) {
  int r;
  jsmn_parser p;
  jsmn_init(&p);
  int token_needed = jsmn_parse(&p, json, strlen(json), NULL, 0);
  //printf("Token needed: %d\n",token_needed);
  jsmntok_t t[token_needed]; 
  jsmn_init(&p);
  r = jsmn_parse(&p, json, strlen(json), t, sizeof(t)/sizeof(t[0]));

  if(checkParseResult(r, t[0]))	
    return t[0].size;
  return -1;
}

char* getValuefromTokens(jsmntok_t t[], int r, const char* key, const char* json) {
  /* Loop over all keys of the root object */
  int i;
  for (i = 1; i < r; i++) {
    if (jsoneq(json, &t[i], key) == 0) {
      /* We may use strndup() to fetch string value */
      char* value = (char*) malloc(t[i+1].end-t[i+1].start);
      sprintf(value,"%.*s", t[i+1].end-t[i+1].start,
          json + t[i+1].start);
      return value;
    } 
  }
  return NULL;
}
