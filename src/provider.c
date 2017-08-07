#include "provider.h"

struct oidc_provider* addProvider(struct oidc_provider* p, size_t* size, struct oidc_provider provider) {
    p = realloc(p, sizeof(struct oidc_provider) * (*size) + sizeof(provider));
    p[*size] = provider;
    (*size)++;
    return p;
}

struct oidc_provider* getProviderFromJSON(char* json) {
  struct oidc_provider* p = NULL;
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
  return NULL;
}

