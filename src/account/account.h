#ifndef ACCOUNT_H
#define ACCOUNT_H

#include "issuer.h"
#include "utils/json.h"
#include "utils/memory.h"
#include "utils/stringUtils.h"

#include "list/list.h"

#include <stdlib.h>
#include <time.h>

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
  time_t              death;
};

char* defineUsableScopes(struct oidc_account account);
inline static struct oidc_issuer* account_getIssuer(struct oidc_account p) {
  return p.issuer;
}
inline static char* account_getIssuerUrl(struct oidc_account p) {
  return p.issuer ? issuer_getIssuerUrl(*(p.issuer)) : NULL;
}
inline static char* account_getConfigEndpoint(struct oidc_account p) {
  return p.issuer ? issuer_getConfigEndpoint(*p.issuer) : NULL;
}
inline static char* account_getTokenEndpoint(struct oidc_account p) {
  return p.issuer ? issuer_getTokenEndpoint(*p.issuer) : NULL;
}
inline static char* account_getAuthorizationEndpoint(struct oidc_account p) {
  return p.issuer ? issuer_getAuthorizationEndpoint(*p.issuer) : NULL;
}
inline static char* account_getRevocationEndpoint(struct oidc_account p) {
  return p.issuer ? issuer_getRevocationEndpoint(*p.issuer) : NULL;
}
inline static char* account_getRegistrationEndpoint(struct oidc_account p) {
  return p.issuer ? issuer_getRegistrationEndpoint(*p.issuer) : NULL;
}
inline static char* account_getDeviceAuthorizationEndpoint(
    struct oidc_account p) {
  return p.issuer ? issuer_getDeviceAuthorizationEndpoint(*p.issuer) : NULL;
}
inline static char* account_getScopesSupported(struct oidc_account p) {
  return p.issuer ? issuer_getScopesSupported(*p.issuer) : NULL;
}
inline static char* account_getGrantTypesSupported(struct oidc_account p) {
  return p.issuer ? issuer_getGrantTypesSupported(*p.issuer) : NULL;
}
inline static char* account_getResponseTypesSupported(struct oidc_account p) {
  return p.issuer ? issuer_getResponseTypesSupported(*p.issuer) : NULL;
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
char*               account_getRefreshToken(struct oidc_account p);
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
inline static time_t account_getDeath(struct oidc_account p) { return p.death; }

inline static void account_setIssuerUrl(struct oidc_account* p,
                                        char*                issuer_url) {
  if (!p->issuer) {
    p->issuer = secAlloc(sizeof(struct oidc_issuer));
  }
  issuer_setIssuerUrl(p->issuer, issuer_url);
}
inline static void account_setClientName(struct oidc_account* p,
                                         char*                clientname) {
  if (p->clientname == clientname) {
    return;
  }
  secFree(p->clientname);
  p->clientname = clientname;
}
inline static void account_setName(struct oidc_account* p, char* shortname,
                                   char* client_identifier) {
  if (p->shortname == shortname) {
    return;
  }
  secFree(p->shortname);
  p->shortname = shortname;
  char* clientname =
      strValid(client_identifier)
          ? oidc_sprintf("oidc-agent:%s-%s", shortname, client_identifier)
          : strValid(shortname) ? oidc_strcat("oidc-agent:", shortname) : NULL;
  account_setClientName(p, clientname);
}
inline static void account_setClientId(struct oidc_account* p,
                                       char*                client_id) {
  if (p->client_id == client_id) {
    return;
  }
  secFree(p->client_id);
  p->client_id = client_id;
}
inline static void account_setClientSecret(struct oidc_account* p,
                                           char*                client_secret) {
  if (p->client_secret == client_secret) {
    return;
  }
  secFree(p->client_secret);
  p->client_secret = client_secret;
}
inline static void account_setScope(struct oidc_account* p, char* scope) {
  if (p->scope == scope) {
    return;
  }
  secFree(p->scope);
  p->scope = scope;
  if (strValid(scope)) {
    char* usable = defineUsableScopes(*p);
    secFree(p->scope);
    p->scope = usable;
  }
}
inline static void account_setIssuer(struct oidc_account* p,
                                     struct oidc_issuer*  issuer) {
  if (p->issuer == issuer) {
    return;
  }
  secFreeIssuer(p->issuer);
  p->issuer = issuer;
  if (issuer) {
    account_setScope(p, defineUsableScopes(*p));
  }
}
inline static void account_setScopesSupported(struct oidc_account* p,
                                              char* scopes_supported) {
  if (!p->issuer) {
    p->issuer = secAlloc(sizeof(struct oidc_issuer));
  }
  if (p->issuer->scopes_supported == scopes_supported) {
    return;
  }
  issuer_setScopesSupported(p->issuer, scopes_supported);
  char* usable = defineUsableScopes(*p);
  secFree(p->scope);
  p->scope = usable;
}
inline static void account_setUsername(struct oidc_account* p, char* username) {
  if (p->username == username) {
    return;
  }
  secFree(p->username);
  p->username = username;
}
inline static void account_setPassword(struct oidc_account* p, char* password) {
  if (p->password == password) {
    return;
  }
  secFree(p->password);
  p->password = password;
}
void account_setRefreshToken(struct oidc_account* p, char* refresh_token);
inline static void account_setAccessToken(struct oidc_account* p,
                                          char*                access_token) {
  if (p->token.access_token == access_token) {
    return;
  }
  secFree(p->token.access_token);
  p->token.access_token = access_token;
}
inline static void account_setTokenExpiresAt(struct oidc_account* p,
                                             unsigned long token_expires_at) {
  if (p->token.token_expires_at == token_expires_at) {
    return;
  }
  p->token.token_expires_at = token_expires_at;
}
inline static void account_setCertPath(struct oidc_account* p,
                                       char*                cert_path) {
  if (p->cert_path == cert_path) {
    return;
  }
  secFree(p->cert_path);
  p->cert_path = cert_path;
}
inline static void account_setRedirectUris(struct oidc_account* p,
                                           list_t*              redirect_uris) {
  if (p->redirect_uris == redirect_uris) {
    return;
  }
  if (p->redirect_uris) {
    list_destroy(p->redirect_uris);
  }
  p->redirect_uris = redirect_uris;
}
inline static void account_setUsedState(struct oidc_account* p,
                                        char*                used_state) {
  if (p->usedState == used_state) {
    return;
  }
  secFree(p->usedState);
  p->usedState = used_state;
}
inline static void account_clearCredentials(struct oidc_account* a) {
  account_setUsername(a, NULL);
  account_setPassword(a, NULL);
}
inline static void account_setDeath(struct oidc_account* p, time_t death) {
  p->death = death;
}
int account_refreshTokenIsValid(struct oidc_account p);

struct oidc_account* getAccountFromJSON(const char* json);
cJSON*               accountToJSON(struct oidc_account p);
char*                accountToJSONString(struct oidc_account p);
cJSON*               accountToJSONWithoutCredentials(struct oidc_account p);
char* accountToJSONStringWithoutCredentials(struct oidc_account p);
void  _secFreeAccount(struct oidc_account* p);
void  secFreeAccountContent(struct oidc_account* p);

int                  accountConfigExists(const char* accountname);
struct oidc_account* decryptAccount(const char* accountname,
                                    const char* password);
struct oidc_account* decryptAccountText(const char* fileText,
                                        const char* password);
char*                getAccountNameList(list_t* accounts);
int                  hasRedirectUris(struct oidc_account account);

int account_matchByState(struct oidc_account* p1, struct oidc_account* p2);
int account_matchByName(struct oidc_account* p1, struct oidc_account* p2);

#ifndef secFreeAccount
#define secFreeAccount(ptr) \
  do {                      \
    _secFreeAccount((ptr)); \
    (ptr) = NULL;           \
  } while (0)
#endif  // secFreeAccount

#endif  // ACCOUNT_H
