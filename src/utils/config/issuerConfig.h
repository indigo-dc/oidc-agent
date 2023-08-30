#ifndef OIDC_AGENT_ISSUERCONFIG_H
#define OIDC_AGENT_ISSUERCONFIG_H

#include "wrapper/list.h"

struct clientConfig {
  char*   client_id;
  char*   client_secret;
  char*   scope;
  list_t* flows;
  list_t* redirect_uris;
};

struct issuerConfig {
  char*                issuer;
  char*                manual_register;
  char*                contact;
  char*                configuration_endpoint;
  char*                device_authorization_endpoint;
  char*                cert_path;
  unsigned char        store_pw : 1;
  unsigned char        store_pw_set : 1;
  unsigned char        oauth : 1;
  unsigned char        oauth_set : 1;
  unsigned char        legacy_aud_mode : 1;
  struct clientConfig* pub_client;
  struct clientConfig* user_client;

  char*   default_account;
  list_t* accounts;
};

const struct issuerConfig* getIssuerConfig(const char* iss);
const list_t*              getPubClientFlows(const char* issuer_url);
const list_t*              getUserClientFlows(const char* issuer_url);
list_t*                    getSuggestableIssuers();
list_t*                    defaultRedirectURIs();
void  oidcp_updateIssuerConfig(const char* action, const char* issuer,
                               const char* shortname);
char* getAccountInfos(list_t* loaded);

#ifndef secFreeIssuerConfig
#define secFreeIssuerConfig(ptr) \
  do {                           \
    _secFreeIssuerConfig((ptr)); \
    (ptr) = NULL;                \
  } while (0)
#endif  // secFreeIssuerConfig

#ifndef secFreePubclientConfig
#define secFreePubclientConfig(ptr) \
  do {                              \
    _secFreePubclientConfig((ptr)); \
    (ptr) = NULL;                   \
  } while (0)
#endif  // secFreePubclientConfig

#endif  // OIDC_AGENT_ISSUERCONFIG_H
