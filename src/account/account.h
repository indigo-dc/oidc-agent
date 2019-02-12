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
  char*               code_challenge_method;
  unsigned char       mode;
};

#define ACCOUNT_MODE_CONFIRM 0x01

char* defineUsableScopes(const struct oidc_account* account);
inline static struct oidc_issuer* account_getIssuer(
    const struct oidc_account* p) {
  return p ? p->issuer : NULL;
}
inline static char* account_getIssuerUrl(const struct oidc_account* p) {
  return p ? p->issuer ? issuer_getIssuerUrl(p->issuer) : NULL : NULL;
}
inline static char* account_getConfigEndpoint(const struct oidc_account* p) {
  return p ? p->issuer ? issuer_getConfigEndpoint(p->issuer) : NULL : NULL;
}
inline static char* account_getTokenEndpoint(const struct oidc_account* p) {
  return p ? p->issuer ? issuer_getTokenEndpoint(p->issuer) : NULL : NULL;
}
inline static char* account_getAuthorizationEndpoint(
    const struct oidc_account* p) {
  return p ? p->issuer ? issuer_getAuthorizationEndpoint(p->issuer) : NULL
           : NULL;
}
inline static char* account_getRevocationEndpoint(
    const struct oidc_account* p) {
  return p ? p->issuer ? issuer_getRevocationEndpoint(p->issuer) : NULL : NULL;
}
inline static char* account_getRegistrationEndpoint(
    const struct oidc_account* p) {
  return p ? p->issuer ? issuer_getRegistrationEndpoint(p->issuer) : NULL
           : NULL;
}
inline static char* account_getDeviceAuthorizationEndpoint(
    const struct oidc_account* p) {
  return p ? p->issuer ? issuer_getDeviceAuthorizationEndpoint(p->issuer) : NULL
           : NULL;
}
inline static char* account_getScopesSupported(const struct oidc_account* p) {
  return p ? p->issuer ? issuer_getScopesSupported(p->issuer) : NULL : NULL;
}
inline static char* account_getGrantTypesSupported(
    const struct oidc_account* p) {
  return p ? p->issuer ? issuer_getGrantTypesSupported(p->issuer) : NULL : NULL;
}
inline static char* account_getResponseTypesSupported(
    const struct oidc_account* p) {
  return p ? p->issuer ? issuer_getResponseTypesSupported(p->issuer) : NULL
           : NULL;
}
inline static char* account_getName(const struct oidc_account* p) {
  return p ? p->shortname : NULL;
}
inline static char* account_getClientName(const struct oidc_account* p) {
  return p ? p->clientname : NULL;
}
inline static char* account_getClientId(const struct oidc_account* p) {
  return p ? p->client_id : NULL;
}
inline static char* account_getClientSecret(const struct oidc_account* p) {
  return p ? p->client_secret : NULL;
}
inline static char* account_getScope(const struct oidc_account* p) {
  return p ? p->scope : NULL;
}
inline static char* account_getUsername(const struct oidc_account* p) {
  return p ? p->username : NULL;
}
inline static char* account_getPassword(const struct oidc_account* p) {
  return p ? p->password : NULL;
}
char*               account_getRefreshToken(const struct oidc_account* p);
inline static char* account_getAccessToken(const struct oidc_account* p) {
  return p ? p->token.access_token : NULL;
}
inline static unsigned long account_getTokenExpiresAt(
    const struct oidc_account* p) {
  return p ? p->token.token_expires_at : 0;
}
inline static char* account_getCertPath(const struct oidc_account* p) {
  return p ? p->cert_path : NULL;
}
inline static list_t* account_getRedirectUris(const struct oidc_account* p) {
  return p ? p->redirect_uris : NULL;
}
inline static size_t account_getRedirectUrisCount(
    const struct oidc_account* p) {
  return p ? p->redirect_uris ? p->redirect_uris->len : 0 : 0;
}
inline static char* account_getUsedState(const struct oidc_account* p) {
  return p ? p->usedState : NULL;
}
inline static time_t account_getDeath(const struct oidc_account* p) {
  return p ? p->death : 0;
}
inline static char* account_getCodeChallengeMethod(
    const struct oidc_account* p) {
  return p ? p->code_challenge_method : NULL;
}
inline static unsigned char account_getConfirmationRequired(
    const struct oidc_account* p) {
  return p ? p->mode & ACCOUNT_MODE_CONFIRM : 0;
}

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
inline static void account_setScopeExact(struct oidc_account* p, char* scope) {
  if (p->scope == scope) {
    return;
  }
  secFree(p->scope);
  p->scope = scope;
}
inline static void account_setScope(struct oidc_account* p, char* scope) {
  account_setScopeExact(p, scope);
  if (strValid(scope)) {
    char* usable = defineUsableScopes(p);
    account_setScopeExact(p, usable);
  }
}
inline static void account_setIssuer(struct oidc_account* p,
                                     struct oidc_issuer*  issuer) {
  if (p->issuer == issuer) {
    return;
  }
  secFreeIssuer(p->issuer);
  p->issuer = issuer;
  if (issuer && strValid(account_getScope(p))) {
    account_setScopeExact(p, defineUsableScopes(p));
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
  char* usable = defineUsableScopes(p);
  account_setScopeExact(p, usable);
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
inline static void account_setCodeChallengeMethod(struct oidc_account* p,
                                                  char* code_challenge_method) {
  if (p->code_challenge_method == code_challenge_method) {
    return;
  }
  secFree(p->code_challenge_method);
  p->code_challenge_method = code_challenge_method;
}
inline static void account_setConfirmationRequired(struct oidc_account* p) {
  p->mode |= ACCOUNT_MODE_CONFIRM;
}

int account_refreshTokenIsValid(const struct oidc_account* p);

struct oidc_account* getAccountFromJSON(const char* json);
cJSON*               accountToJSON(const struct oidc_account* p);
char*                accountToJSONString(const struct oidc_account* p);
cJSON* accountToJSONWithoutCredentials(const struct oidc_account* p);
char*  accountToJSONStringWithoutCredentials(const struct oidc_account* p);
void   _secFreeAccount(struct oidc_account* p);
void   secFreeAccountContent(struct oidc_account* p);

struct oidc_account* updateAccountWithPublicClientInfo(struct oidc_account*);
int                  accountConfigExists(const char* accountname);
struct oidc_account* decryptAccount(const char* accountname,
                                    const char* password);
struct oidc_account* decryptAccountText(const char* fileText,
                                        const char* password);
char*                getAccountNameList(list_t* accounts);
int                  hasRedirectUris(const struct oidc_account* account);

int account_matchByState(const struct oidc_account* p1,
                         const struct oidc_account* p2);
int account_matchByName(const struct oidc_account* p1,
                        const struct oidc_account* p2);

#ifndef secFreeAccount
#define secFreeAccount(ptr) \
  do {                      \
    _secFreeAccount((ptr)); \
    (ptr) = NULL;           \
  } while (0)
#endif  // secFreeAccount

#endif  // ACCOUNT_H
