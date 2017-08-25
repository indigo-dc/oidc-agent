#ifndef PROVIDER_H
#define PROVIDER_H

#include "json.h"
#include "oidc_utilities.h"

struct token {
  char* access_token;
  unsigned long token_expires_at;
};

struct oidc_provider {
  char* issuer;
  char* name;                           
  char* client_id;                     
  char* client_secret;                
  char* configuration_endpoint;      
  char* token_endpoint;             
  char* authorization_endpoint;
  char* revocation_endpoint;
  char* registration_endpoint;
  char* username;                  
  char* password;
  char* refresh_token;            
  struct token token;
  char* cert_path;
};

inline static char* provider_getIssuer(struct oidc_provider p) { return p.issuer; }
inline static char* provider_getName(struct oidc_provider p) { return p.name; }
inline static char* provider_getClientId(struct oidc_provider p) { return p.client_id; }
inline static char* provider_getClientSecret(struct oidc_provider p) { return p.client_secret; }
inline static char* provider_getConfigEndpoint(struct oidc_provider p) { return p.configuration_endpoint; }
inline static char* provider_getTokenEndpoint(struct oidc_provider p) { return p.token_endpoint; }
inline static char* provider_getAuthorizationEndpoint(struct oidc_provider p) { return p.authorization_endpoint; }
inline static char* provider_getRevocationEndpoint(struct oidc_provider p) { return p.revocation_endpoint; }
inline static char* provider_getRegistrationEndpoint(struct oidc_provider p) { return p.registration_endpoint; }
inline static char* provider_getUsername(struct oidc_provider p) { return p.username; }
inline static char* provider_getPassword(struct oidc_provider p) { return p.password; }
inline static char* provider_getRefreshToken(struct oidc_provider p) { return p.refresh_token; }
inline static char* provider_getAccessToken(struct oidc_provider p) { return p.token.access_token; }
inline static unsigned long provider_getTokenExpiresAt(struct oidc_provider p) { return p.token.token_expires_at; }
inline static char* provider_getCertPath(struct oidc_provider p) { return p.cert_path; }

inline static void provider_setIssuer(struct oidc_provider* p, char* issuer) { clearFreeString(p->issuer); p->issuer=issuer; }
inline static void provider_setName(struct oidc_provider* p, char* name) { clearFreeString(p->name); p->name=name; }
inline static void provider_setClientId(struct oidc_provider* p, char* client_id) { clearFreeString(p->client_id); p->client_id=client_id; }
inline static void provider_setClientSecret(struct oidc_provider* p, char* client_secret) { clearFreeString(p->client_secret); p->client_secret=client_secret; }
inline static void provider_setConfigEndpoint(struct oidc_provider* p, char* configuration_endpoint) { clearFreeString(p->configuration_endpoint); p->configuration_endpoint=configuration_endpoint; }
inline static void provider_setTokenEndpoint(struct oidc_provider* p, char* token_endpoint) { clearFreeString(p->token_endpoint); p->token_endpoint=token_endpoint; }
inline static void provider_setAuthorizationEndpoint(struct oidc_provider* p, char* authorization_endpoint) { clearFreeString(p->authorization_endpoint); p->authorization_endpoint=authorization_endpoint; }
inline static void provider_setRevocationEndpoint(struct oidc_provider* p, char* revocation_endpoint) { clearFreeString(p->revocation_endpoint); p->revocation_endpoint=revocation_endpoint; }
inline static void provider_setRegistrationEndpoint(struct oidc_provider* p, char* registration_endpoint) { clearFreeString(p->registration_endpoint); p->registration_endpoint=registration_endpoint; }
inline static void provider_setUsername(struct oidc_provider* p, char* username) { clearFreeString(p->username); p->username=username; }
inline static void provider_setPassword(struct oidc_provider* p, char* password) { clearFreeString(p->password); p->password=password; }
inline static void provider_setRefreshToken(struct oidc_provider* p, char* refresh_token) { clearFreeString(p->refresh_token); p->refresh_token=refresh_token; }
inline static void provider_setAccessToken(struct oidc_provider* p, char* access_token) { clearFreeString(p->token.access_token); p->token.access_token=access_token; }
inline static void provider_setTokenExpiresAt(struct oidc_provider* p, unsigned long token_expires_at) { p->token.token_expires_at=token_expires_at; }
inline static void provider_setCertPath(struct oidc_provider* p, char* cert_path) { clearFreeString(p->cert_path); p->cert_path=cert_path; }


struct oidc_provider* addProvider(struct oidc_provider* p, size_t* size, struct oidc_provider provider) ;
struct oidc_provider* findProvider(struct oidc_provider* p, size_t size, struct oidc_provider key) ;
struct oidc_provider* removeProvider(struct oidc_provider* p, size_t* size, struct oidc_provider key) ;
struct oidc_provider* getProviderFromJSON(char* json) ;
char* providerToJSON(struct oidc_provider p) ;
void freeProvider(struct oidc_provider* p) ;
void freeProviderContent(struct oidc_provider* p) ;

int providerConfigExists(const char* providername) ;
struct oidc_provider* decryptProvider(const char* providername, const char* password) ;
char* getProviderNameList(struct oidc_provider* p, size_t size) ;

#endif // PROVIDER_H

