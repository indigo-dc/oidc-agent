#ifndef OIDC_AGENT_ISSUERCONFIG_H
#define OIDC_AGENT_ISSUERCONFIG_H

#include "wrapper/list.h"

struct pubclientConfig {
  char*   client_id;
  char*   client_secret;
  char*   scope;
  list_t* flows;
};

struct issuerConfig {
  char*                   issuer;
  char*                   manual_register;
  char*                   contact;
  char*                   configuration_endpoint;
  char*                   device_authorization_endpoint;
  char*                   cert_path;
  unsigned char           store_pw;
  unsigned char           store_pw_set;
  unsigned char           oauth;
  unsigned char           oauth_set;
  struct pubclientConfig* pubclient;

  char*   default_account;
  list_t* accounts;
};

const struct issuerConfig* getIssuerConfig(const char* iss);
list_t*                    getSuggestableIssuers();

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
