#ifndef ACCOUNT_H
#define ACCOUNT_H

#include <stdlib.h>
#include <time.h>

#include "issuer.h"
#include "utils/file_io/promptCryptFileUtils.h"
#include "wrapper/cjson.h"
#include "wrapper/list.h"

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
  char*               audience;
  char*               used_mytoken_profile;
  char*               username;
  char*               password;
  char*               refresh_token;
  struct token        token;
  char*               cert_path;
  list_t*             redirect_uris;
  char*               usedState;
  unsigned char       usedStateChecked;
  time_t              death;
  char*               code_challenge_method;
  unsigned char       mode;
};

#define ACCOUNT_MODE_CONFIRM 0x01
#define ACCOUNT_MODE_NO_WEBSERVER 0x02
#define ACCOUNT_MODE_NO_SCHEME 0x04
#define ACCOUNT_MODE_ALWAYSALLOWID 0x08
#define ACCOUNT_MODE_OAUTH2 0x10
#define ACCOUNT_MODE_PUBCLIENT 0x20
#define ACCOUNT_MODE_UNUSED 0x40
#define ACCOUNT_MODE_UNUSED_ 0x80

char*                defineUsableScopes(const struct oidc_account* account);
struct oidc_account* getAccountFromJSON(const char* json);
cJSON*               accountToJSON(const struct oidc_account* p);
char*                accountToJSONString(const struct oidc_account* p);
cJSON* accountToJSONWithoutCredentials(const struct oidc_account* p);
char*  accountToJSONStringWithoutCredentials(const struct oidc_account* p);
void   _secFreeAccount(struct oidc_account* p);
void   secFreeAccountContent(struct oidc_account* p);

struct oidc_account* updateAccountWithPublicClientInfo(struct oidc_account*);
struct oidc_account* updateAccountWithUserClientInfo(struct oidc_account*);
char*                getScopesForPublicClient(const struct oidc_account*);
char*                getScopesForUserClient(const struct oidc_account*);
int                  accountConfigExists(const char* accountname);
char*                getAccountNameList(list_t* accounts);
int                  hasRedirectUris(const struct oidc_account* account);

int   account_matchByState(const struct oidc_account* p1,
                           const struct oidc_account* p2);
int   account_matchByName(const struct oidc_account* p1,
                          const struct oidc_account* p2);
int   account_matchByIssuerUrl(const struct oidc_account* p1,
                               const struct oidc_account* p2);
char* getOSDefaultCertPath();

// make setters and getters avialable
#include "account/setandget.h"

#ifndef secFreeAccount
#define secFreeAccount(ptr) \
  do {                      \
    _secFreeAccount((ptr)); \
    (ptr) = NULL;           \
  } while (0)
#endif  // secFreeAccount

#endif  // ACCOUNT_H
