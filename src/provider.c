#include <stdio.h>

#include "provider.h"

struct oidc_provider* addProvider(struct oidc_provider* p, size_t* size, struct oidc_provider provider) {
    p = realloc(p, sizeof(struct oidc_provider) * (*size) + sizeof(provider));
    p[*size] = provider;
    (*size)++;
    return p;
}

struct oidc_provider* getProviderFromJSON(char* json) {
  if(NULL==json)
    return NULL;
  struct oidc_provider* p = calloc(sizeof(struct oidc_provider), 1);
  struct key_value pairs[9];
  pairs[0].key = "issuer";
  pairs[1].key = "name";
  pairs[2].key = "client_id";
  pairs[3].key = "client_secret";
  pairs[4].key = "configuration_endpoint";
  pairs[5].key = "token_endpoint";
  pairs[6].key = "username";
  pairs[7].key = "password";
  pairs[8].key = "refresh_token";
  if(getJSONValues(json, pairs, sizeof(pairs)/sizeof(*pairs))>0) {
  provider_setIssuer(p, pairs[0].value);
  provider_setName(p, pairs[1].value);
  provider_setClientId(p, pairs[2].value);
  provider_setClientSecret(p, pairs[3].value);
  provider_setConfigEndpoint(p, pairs[4].value);
  provider_setTokenEndpoint(p, pairs[5].value);
  provider_setUsername(p, pairs[6].value);
  provider_setPassword(p, pairs[7].value);
  provider_setRefreshToken(p, pairs[8].value);
      } 
  if(p->issuer && p->name && p->client_id && p->client_secret && p->configuration_endpoint && p->token_endpoint && p->username && p->password && p->refresh_token) 
        return p;
  free(p);
  return NULL;
}

char* providerToJSON(struct oidc_provider p) {
  char* fmt = "{\n\"name\":\"%s\",\n\"issuer\":\"%s\",,\n\"configuration_endpoint\":\"%s\",\n\"token_endpoint\":\"%s\",\n\"client_id\":\"%s\",\n\"client_secret\":\"%s\",\n\"username\":\"%s\",\n\"password\":\"%s\",\n\"refresh_token\":\"%s\"\n}";
  char* p_json = calloc(sizeof(char), snprintf(NULL, 0, fmt, p.name, p.issuer, p.configuration_endpoint, p.token_endpoint, p.client_id, p.client_secret, p.username, p.password, p.refresh_token)+1);
  sprintf(p_json, fmt, p.name, p.issuer, p.configuration_endpoint, p.token_endpoint, p.client_id, p.client_secret, p.username, p.password, p.refresh_token);
  return p_json;
}

