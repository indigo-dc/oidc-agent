#ifndef ACCOUNT_H
#define ACCOUNT_H

#include "json.h"
#include "oidc_utilities.h"

struct token {
  char* access_token;
  unsigned long token_expires_at;
};

struct oidc_account {
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

inline static char* account_getIssuer(struct oidc_account p) { return p.issuer; }
inline static char* account_getName(struct oidc_account p) { return p.name; }
inline static char* account_getClientId(struct oidc_account p) { return p.client_id; }
inline static char* account_getClientSecret(struct oidc_account p) { return p.client_secret; }
inline static char* account_getConfigEndpoint(struct oidc_account p) { return p.configuration_endpoint; }
inline static char* account_getTokenEndpoint(struct oidc_account p) { return p.token_endpoint; }
inline static char* account_getAuthorizationEndpoint(struct oidc_account p) { return p.authorization_endpoint; }
inline static char* account_getRevocationEndpoint(struct oidc_account p) { return p.revocation_endpoint; }
inline static char* account_getRegistrationEndpoint(struct oidc_account p) { return p.registration_endpoint; }
inline static char* account_getUsername(struct oidc_account p) { return p.username; }
inline static char* account_getPassword(struct oidc_account p) { return p.password; }
inline static char* account_getRefreshToken(struct oidc_account p) { return p.refresh_token; }
inline static char* account_getAccessToken(struct oidc_account p) { return p.token.access_token; }
inline static unsigned long account_getTokenExpiresAt(struct oidc_account p) { return p.token.token_expires_at; }
inline static char* account_getCertPath(struct oidc_account p) { return p.cert_path; }

inline static void account_setIssuer(struct oidc_account* p, char* issuer) { clearFreeString(p->issuer); p->issuer=issuer; }
inline static void account_setName(struct oidc_account* p, char* name) { clearFreeString(p->name); p->name=name; }
inline static void account_setClientId(struct oidc_account* p, char* client_id) { clearFreeString(p->client_id); p->client_id=client_id; }
inline static void account_setClientSecret(struct oidc_account* p, char* client_secret) { clearFreeString(p->client_secret); p->client_secret=client_secret; }
inline static void account_setConfigEndpoint(struct oidc_account* p, char* configuration_endpoint) { clearFreeString(p->configuration_endpoint); p->configuration_endpoint=configuration_endpoint; }
inline static void account_setTokenEndpoint(struct oidc_account* p, char* token_endpoint) { clearFreeString(p->token_endpoint); p->token_endpoint=token_endpoint; }
inline static void account_setAuthorizationEndpoint(struct oidc_account* p, char* authorization_endpoint) { clearFreeString(p->authorization_endpoint); p->authorization_endpoint=authorization_endpoint; }
inline static void account_setRevocationEndpoint(struct oidc_account* p, char* revocation_endpoint) { clearFreeString(p->revocation_endpoint); p->revocation_endpoint=revocation_endpoint; }
inline static void account_setRegistrationEndpoint(struct oidc_account* p, char* registration_endpoint) { clearFreeString(p->registration_endpoint); p->registration_endpoint=registration_endpoint; }
inline static void account_setUsername(struct oidc_account* p, char* username) { clearFreeString(p->username); p->username=username; }
inline static void account_setPassword(struct oidc_account* p, char* password) { clearFreeString(p->password); p->password=password; }
inline static void account_setRefreshToken(struct oidc_account* p, char* refresh_token) { clearFreeString(p->refresh_token); p->refresh_token=refresh_token; }
inline static void account_setAccessToken(struct oidc_account* p, char* access_token) { clearFreeString(p->token.access_token); p->token.access_token=access_token; }
inline static void account_setTokenExpiresAt(struct oidc_account* p, unsigned long token_expires_at) { p->token.token_expires_at=token_expires_at; }
inline static void account_setCertPath(struct oidc_account* p, char* cert_path) { clearFreeString(p->cert_path); p->cert_path=cert_path; }


struct oidc_account* addAccount(struct oidc_account* p, size_t* size, struct oidc_account account) ;
struct oidc_account* findAccount(struct oidc_account* p, size_t size, struct oidc_account key) ;
struct oidc_account* removeAccount(struct oidc_account* p, size_t* size, struct oidc_account key) ;
struct oidc_account* getAccountFromJSON(char* json) ;
char* accountToJSON(struct oidc_account p) ;
void freeAccount(struct oidc_account* p) ;
void freeAccountContent(struct oidc_account* p) ;

int accountConfigExists(const char* accountname) ;
struct oidc_account* decryptAccount(const char* accountname, const char* password) ;
struct oidc_account* decryptAccountText(char* fileText, const char* password) ;
char* getAccountNameList(struct oidc_account* p, size_t size) ;

#endif // ACCOUNT_H

