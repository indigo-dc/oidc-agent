#ifndef PROVIDER_H
#define PROVIDER_H

#include <stdlib.h>

#include "json.h"

struct token {
  char* access_token;
  unsigned long token_expires_at;
};

struct oidc_provider {
  char* issuer;
  char* name;                           // mandatory in config file
  char* client_id;                      // mandatory in config file
  char* client_secret;                  // mandatory in config file
  char* configuration_endpoint;         // mandatory in config file
  char* token_endpoint;                 // retrieved from configuration_endpoint
  char* username;                       // optional in config file
  char* password;
  char* refresh_token;                  // optional in config file
  struct token token;
};

inline static char* provider_getIssuer(struct oidc_provider p) { return p.issuer; }
inline static char* provider_getName(struct oidc_provider p) { return p.name; }
inline static char* provider_getClientId(struct oidc_provider p) { return p.client_id; }
inline static char* provider_getClientSecret(struct oidc_provider p) { return p.client_secret; }
inline static char* provider_getConfigEndpoint(struct oidc_provider p) { return p.configuration_endpoint; }
inline static char* provider_getTokenEndpoint(struct oidc_provider p) { return p.token_endpoint; }
inline static char* provider_getUsername(struct oidc_provider p) { return p.username; }
inline static char* provider_getPassword(struct oidc_provider p) { return p.password; }
inline static char* provider_getRefreshToken(struct oidc_provider p) { return p.refresh_token; }
inline static char* provider_getAccessToken(struct oidc_provider p) { return p.token.access_token; }
inline static unsigned long provider_getTokenExpiresAt(struct oidc_provider p) { return p.token.token_expires_at; }

inline static void provider_setIssuer(struct oidc_provider* p, char* issuer) { free(p->issuer); p->issuer=issuer; }
inline static void provider_setName(struct oidc_provider* p, char* name) { free(p->name); p->name=name; }
inline static void provider_setClientId(struct oidc_provider* p, char* client_id) { free(p->client_id); p->client_id=client_id; }
inline static void provider_setClientSecret(struct oidc_provider* p, char* client_secret) { free(p->client_secret); p->client_secret=client_secret; }
inline static void provider_setConfigEndpoint(struct oidc_provider* p, char* configuration_endpoint) { free(p->configuration_endpoint); p->configuration_endpoint=configuration_endpoint; }
inline static void provider_setTokenEndpoint(struct oidc_provider* p, char* token_endpoint) { free(p->token_endpoint); p->token_endpoint=token_endpoint; }
inline static void provider_setUsername(struct oidc_provider* p, char* username) { free(p->username); p->username=username; }
inline static void provider_setPassword(struct oidc_provider* p, char* password) { free(p->password); p->password=password; }
inline static void provider_setRefreshToken(struct oidc_provider* p, char* refresh_token) { free(p->refresh_token); p->refresh_token=refresh_token; }
inline static void provider_setAccessToken(struct oidc_provider* p, char* access_token) { free(p->token.access_token); p->token.access_token=access_token; }
inline static void provider_setTokenExpiresAt(struct oidc_provider* p, unsigned long token_expires_at) { p->token.token_expires_at=token_expires_at; }


struct oidc_provider* addProvider(struct oidc_provider* p, size_t* size, struct oidc_provider provider) ;
struct oidc_provider* getProviderFromJSON(char* json) ;


#endif // PROVIDER_H

