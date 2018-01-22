#ifndef ACCOUNT_H
#define ACCOUNT_H

#include "json.h"
#include "issuer.h"
#include "oidc_utilities.h"

#include <stdlib.h>

struct token {
  char* access_token;
  unsigned long token_expires_at;
};

struct oidc_account {
  struct oidc_issuer* issuer;
  char* name;                           
  char* client_id;                     
  char* client_secret;                
  char* scope;
  char* username;                  
  char* password;
  char* refresh_token;            
  struct token token;
  char* cert_path;
  char** redirect_uris;
  size_t redirect_uris_count;
  char* usedState;
};

char* defineUsableScopes(struct oidc_account account) ;
inline static struct oidc_issuer* account_getIssuer(struct oidc_account p) { return p.issuer; }
inline static char* account_getIssuerUrl(struct oidc_account p) {return p.issuer ? issuer_getIssuerUrl(*(p.issuer)) : NULL; }
inline static char* account_getConfigEndpoint(struct oidc_account p) { return issuer_getConfigEndpoint(*p.issuer); }
inline static char* account_getTokenEndpoint(struct oidc_account p) { return issuer_getTokenEndpoint(*p.issuer); }
inline static char* account_getAuthorizationEndpoint(struct oidc_account p) { return issuer_getAuthorizationEndpoint(*p.issuer); }
inline static char* account_getRevocationEndpoint(struct oidc_account p) { return issuer_getRevocationEndpoint(*p.issuer); }
inline static char* account_getRegistrationEndpoint(struct oidc_account p) { return issuer_getRegistrationEndpoint(*p.issuer); }
inline static char* account_getScopesSupported(struct oidc_account p) { return issuer_getScopesSupported(*p.issuer); }
inline static char* account_getGrantTypesSupported(struct oidc_account p) { return issuer_getGrantTypesSupported(*p.issuer); }
inline static char* account_getResponseTypesSupported(struct oidc_account p) { return issuer_getResponseTypesSupported(*p.issuer); }
inline static char* account_getName(struct oidc_account p) { return p.name; }
inline static char* account_getClientId(struct oidc_account p) { return p.client_id; }
inline static char* account_getClientSecret(struct oidc_account p) { return p.client_secret; }
inline static char* account_getScope(struct oidc_account p) { return p.scope; }
inline static char* account_getUsername(struct oidc_account p) { return p.username; }
inline static char* account_getPassword(struct oidc_account p) { return p.password; }
inline static char* account_getRefreshToken(struct oidc_account p) { return p.refresh_token; }
inline static char* account_getAccessToken(struct oidc_account p) { return p.token.access_token; }
inline static unsigned long account_getTokenExpiresAt(struct oidc_account p) { return p.token.token_expires_at; }
inline static char* account_getCertPath(struct oidc_account p) { return p.cert_path; }
inline static char** account_getRedirectUris(struct oidc_account p) { return p.redirect_uris; }
inline static size_t account_getRedirectUrisCount(struct oidc_account p) { return p.redirect_uris_count; }
inline static char* account_getUsedState(struct oidc_account p) { return p.usedState; }

inline static void account_setIssuerUrl(struct oidc_account* p, char* issuer_url) { if(!p->issuer) { p->issuer = calloc(sizeof(struct oidc_issuer), 1); } issuer_setIssuerUrl(p->issuer, issuer_url);}
inline static void account_setName(struct oidc_account* p, char* name) { clearFreeString(p->name); p->name=name; }
inline static void account_setClientId(struct oidc_account* p, char* client_id) { clearFreeString(p->client_id); p->client_id=client_id; }
inline static void account_setClientSecret(struct oidc_account* p, char* client_secret) { clearFreeString(p->client_secret); p->client_secret=client_secret; }
inline static void account_setScope(struct oidc_account* p, char* scope) { 
  clearFreeString(p->scope); p->scope=scope; 
  if(isValid(scope)) {
  char* usable = defineUsableScopes(*p);
  clearFreeString(p->scope); p->scope=usable;
  }
}
inline static void account_setIssuer(struct oidc_account* p, struct oidc_issuer* issuer) { clearFreeIssuer(p->issuer); p->issuer=issuer; if(issuer) {account_setScope(p, defineUsableScopes(*p));}}
inline static void account_setScopesSupported(struct oidc_account* p, char* scopes_supported) {
  issuer_setScopesSupported(p->issuer, scopes_supported);
  char* usable = defineUsableScopes(*p);
  clearFreeString(p->scope); p->scope=usable;
}
inline static void account_setUsername(struct oidc_account* p, char* username) { clearFreeString(p->username); p->username=username; }
inline static void account_setPassword(struct oidc_account* p, char* password) { clearFreeString(p->password); p->password=password; }
inline static void account_setRefreshToken(struct oidc_account* p, char* refresh_token) { clearFreeString(p->refresh_token); p->refresh_token=refresh_token; }
inline static void account_setAccessToken(struct oidc_account* p, char* access_token) { clearFreeString(p->token.access_token); p->token.access_token=access_token; }
inline static void account_setTokenExpiresAt(struct oidc_account* p, unsigned long token_expires_at) { p->token.token_expires_at=token_expires_at; }
inline static void account_setCertPath(struct oidc_account* p, char* cert_path) { clearFreeString(p->cert_path); p->cert_path=cert_path; }
inline static void account_setRedirectUris(struct oidc_account* p, char** redirect_uris, size_t redirect_uris_count) { 
  size_t i;
  for(i=0; i < p->redirect_uris_count; i++) {
    clearFreeString(*(p->redirect_uris + i));
  }
  clearFree(p->redirect_uris, sizeof(char*) * p->redirect_uris_count);
  p->redirect_uris = redirect_uris;
  p->redirect_uris_count = redirect_uris_count;
}
inline static void account_setUsedState(struct oidc_account* p, char* used_state) { clearFreeString(p->usedState); p->usedState=used_state; }

struct oidc_account* addAccount(struct oidc_account* p, size_t* size, struct oidc_account account) ;
struct oidc_account* findAccountByName(struct oidc_account* p, size_t size, struct oidc_account key) ;
struct oidc_account* findAccountByState(struct oidc_account* p, size_t size, struct oidc_account key) ;
struct oidc_account* removeAccount(struct oidc_account* p, size_t* size, struct oidc_account key) ;
struct oidc_account* getAccountFromJSON(char* json) ;
char* accountToJSON(struct oidc_account p) ;
void freeAccount(struct oidc_account* p) ;
void freeAccountContent(struct oidc_account* p) ;

int accountConfigExists(const char* accountname) ;
struct oidc_account* decryptAccount(const char* accountname, const char* password) ;
struct oidc_account* decryptAccountText(char* fileText, const char* password) ;
char* getAccountNameList(struct oidc_account* p, size_t size) ;
int hasRedirectUris(struct oidc_account account) ;

#endif // ACCOUNT_H

