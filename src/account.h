#ifndef ACCOUNT_H
#define ACCOUNT_H

#include "issuer.h"
#include "json.h"
#include "utils/cleaner.h"

#include "../lib/list/src/list.h"

#include <stdlib.h>

struct token {
  char*         access_token;
  unsigned long token_expires_at;
};

struct oidc_account {
  struct oidc_issuer* issuer;
  char*               shortname;
  char*               clientname;
  char*               client_id;
  char*               client_secret;
  char*               scope;
  char*               username;
  char*               password;
  char*               refresh_token;
  struct token        token;
  char*               cert_path;
  list_t*             redirect_uris;
  char*               usedState;
};

char* defineUsableScopes(struct oidc_account account);
inline static struct oidc_issuer* account_getIssuer(struct oidc_account p) {
  return p.issuer;
}
inline static char* account_getIssuerUrl(struct oidc_account p) {
  return p.issuer ? issuer_getIssuerUrl(*(p.issuer)) : NULL;
}
inline static char* account_getConfigEndpoint(struct oidc_account p) {
  return issuer_getConfigEndpoint(*p.issuer);
}
inline static char* account_getTokenEndpoint(struct oidc_account p) {
  return issuer_getTokenEndpoint(*p.issuer);
}
inline static char* account_getAuthorizationEndpoint(struct oidc_account p) {
  return issuer_getAuthorizationEndpoint(*p.issuer);
}
inline static char* account_getRevocationEndpoint(struct oidc_account p) {
  return issuer_getRevocationEndpoint(*p.issuer);
}
inline static char* account_getRegistrationEndpoint(struct oidc_account p) {
  return issuer_getRegistrationEndpoint(*p.issuer);
}
inline static char* account_getDeviceAuthorizationEndpoint(
    struct oidc_account p) {
  return issuer_getDeviceAuthorizationEndpoint(*p.issuer);
}
inline static char* account_getScopesSupported(struct oidc_account p) {
  return issuer_getScopesSupported(*p.issuer);
}
inline static char* account_getGrantTypesSupported(struct oidc_account p) {
  return issuer_getGrantTypesSupported(*p.issuer);
}
inline static char* account_getResponseTypesSupported(struct oidc_account p) {
  return issuer_getResponseTypesSupported(*p.issuer);
}
inline static char* account_getName(struct oidc_account p) {
  return p.shortname;
}
inline static char* account_getClientName(struct oidc_account p) {
  return p.clientname;
}
inline static char* account_getClientId(struct oidc_account p) {
  return p.client_id;
}
inline static char* account_getClientSecret(struct oidc_account p) {
  return p.client_secret;
}
inline static char* account_getScope(struct oidc_account p) { return p.scope; }
inline static char* account_getUsername(struct oidc_account p) {
  return p.username;
}
inline static char* account_getPassword(struct oidc_account p) {
  return p.password;
}
inline static char* account_getRefreshToken(struct oidc_account p) {
  return p.refresh_token;
}
inline static char* account_getAccessToken(struct oidc_account p) {
  return p.token.access_token;
}
inline static unsigned long account_getTokenExpiresAt(struct oidc_account p) {
  return p.token.token_expires_at;
}
inline static char* account_getCertPath(struct oidc_account p) {
  return p.cert_path;
}
inline static list_t* account_getRedirectUris(struct oidc_account p) {
  return p.redirect_uris;
}
inline static size_t account_getRedirectUrisCount(struct oidc_account p) {
  return p.redirect_uris ? p.redirect_uris->len : 0;
}
inline static char* account_getUsedState(struct oidc_account p) {
  return p.usedState;
}

inline static void account_setIssuerUrl(struct oidc_account* p,
                                        char*                issuer_url) {
  if (!p->issuer) {
    p->issuer = calloc(sizeof(struct oidc_issuer), 1);
  }
  issuer_setIssuerUrl(p->issuer, issuer_url);
}
inline static void account_setClientName(struct oidc_account* p,
                                         char*                clientname) {
  clearFreeString(p->clientname);
  p->clientname = clientname;
}
inline static void account_setName(struct oidc_account* p, char* shortname,
                                   char* client_identifier) {
  clearFreeString(p->shortname);
  p->shortname = shortname;
  char* clientname =
      strValid(client_identifier)
          ? oidc_sprintf("oidc-agent:%s-%s", shortname, client_identifier)
          : oidc_strcat("oidc-agent:", shortname);
  account_setClientName(p, clientname);
}
inline static void account_setClientId(struct oidc_account* p,
                                       char*                client_id) {
  clearFreeString(p->client_id);
  p->client_id = client_id;
}
inline static void account_setClientSecret(struct oidc_account* p,
                                           char*                client_secret) {
  clearFreeString(p->client_secret);
  p->client_secret = client_secret;
}
inline static void account_setScope(struct oidc_account* p, char* scope) {
  clearFreeString(p->scope);
  p->scope = scope;
  if (strValid(scope)) {
    char* usable = defineUsableScopes(*p);
    clearFreeString(p->scope);
    p->scope = usable;
  }
}
inline static void account_setIssuer(struct oidc_account* p,
                                     struct oidc_issuer*  issuer) {
  clearFreeIssuer(p->issuer);
  p->issuer = issuer;
  if (issuer) {
    account_setScope(p, defineUsableScopes(*p));
  }
}
inline static void account_setScopesSupported(struct oidc_account* p,
                                              char* scopes_supported) {
  issuer_setScopesSupported(p->issuer, scopes_supported);
  char* usable = defineUsableScopes(*p);
  clearFreeString(p->scope);
  p->scope = usable;
}
inline static void account_setUsername(struct oidc_account* p, char* username) {
  clearFreeString(p->username);
  p->username = username;
}
inline static void account_setPassword(struct oidc_account* p, char* password) {
  clearFreeString(p->password);
  p->password = password;
}
inline static void account_setRefreshToken(struct oidc_account* p,
                                           char*                refresh_token) {
  clearFreeString(p->refresh_token);
  p->refresh_token = refresh_token;
}
inline static void account_setAccessToken(struct oidc_account* p,
                                          char*                access_token) {
  clearFreeString(p->token.access_token);
  p->token.access_token = access_token;
}
inline static void account_setTokenExpiresAt(struct oidc_account* p,
                                             unsigned long token_expires_at) {
  p->token.token_expires_at = token_expires_at;
}
inline static void account_setCertPath(struct oidc_account* p,
                                       char*                cert_path) {
  clearFreeString(p->cert_path);
  p->cert_path = cert_path;
}
inline static void account_setRedirectUris(struct oidc_account* p,
                                           list_t*              redirect_uris) {
  if (p->redirect_uris) {
    list_destroy(p->redirect_uris);
  }
  p->redirect_uris = redirect_uris;
}
inline static void account_setUsedState(struct oidc_account* p,
                                        char*                used_state) {
  clearFreeString(p->usedState);
  p->usedState = used_state;
}
inline static void account_clearCredentials(struct oidc_account* a) {
  account_setUsername(a, NULL);
  account_setPassword(a, NULL);
}

struct oidc_account* getAccountFromJSON(char* json);
char*                accountToJSON(struct oidc_account p);
char*                accountToJSONWithoutCredentials(struct oidc_account p);
void                 clearFreeAccount(struct oidc_account* p);
void                 clearFreeAccountContent(struct oidc_account* p);

int                  accountConfigExists(const char* accountname);
struct oidc_account* decryptAccount(const char* accountname,
                                    const char* password);
struct oidc_account* decryptAccountText(char* fileText, const char* password);
char*                getAccountNameList(list_t* accounts);
int                  hasRedirectUris(struct oidc_account account);

int account_matchByState(struct oidc_account* p1, struct oidc_account* p2);
int account_matchByName(struct oidc_account* p1, struct oidc_account* p2);

#endif  // ACCOUNT_H
