#ifndef OIDC_GEN_OPTIONS_H
#define OIDC_GEN_OPTIONS_H

#include <argp.h>

#include "wrapper/list.h"

#define OPT_LONG_CLIENTID "client-id"
#define OPT_LONG_CLIENTSECRET "client-secret"
#define OPT_LONG_REFRESHTOKEN "rt"
#define OPT_LONG_REFRESHTOKEN_ENV "rt-env"
#define OPT_LONG_USERNAME "op-username"
#define OPT_LONG_PASSWORD "op-password"
#define OPT_LONG_CERTPATH "cert-path"
#define OPT_LONG_ISSUER "issuer"
#define OPT_LONG_AUDIENCE "aud"
#define OPT_LONG_SCOPE "scope"
#define OPT_LONG_REDIRECT "redirect-uri"
#define OPT_LONG_DEVICE "dae"
#define OPT_LONG_CONFIG_ENDPOINT "configuration-endpoint"
#define OPT_LONG_OAUTH2 "oauth2"
#define OPT_LONG_MYTOKENURL "mytoken-url"
#define OPT_LONG_MYTOKENPROFILE "mytoken"

struct optional_arg {
  char* str;
  short useIt;
};

struct arguments {
  char* args[1]; /* account */
  char* print;
  char* rename;
  char* updateConfigFile;
  char* codeExchange;
  char* state;
  char* device_authorization_endpoint;
  char* configuration_endpoint;
  char* pw_cmd;
  char* pw_file;
  char* pw_env;
  char* pw_gpg;
  char* file;

  char* client_id;
  char* client_secret;
  char* issuer;
  char* redirect_uri;
  char* scope;
  char* dynRegToken;
  char* cert_path;
  char* refresh_token;
  char* cnid;
  char* audience;
  char* op_username;
  char* op_password;
  char* mytoken_profile;

  struct optional_arg mytoken_issuer;

  list_t*       flows;
  unsigned char flows_set;
  list_t*       redirect_uris;

  unsigned char delete;
  unsigned char listAccounts;
  unsigned char reauthenticate;
  unsigned char manual;
  unsigned char usePublicClient;
  unsigned char noUrlCall;
  unsigned char noWebserver;
  unsigned char noScheme;
  unsigned char pw_prompt_mode;
  unsigned char prompt_mode;
  unsigned char debug;
  unsigned char verbose;
  unsigned char confirm_yes;
  unsigned char confirm_no;
  unsigned char confirm_default;
  unsigned char only_at;
  unsigned char noSave;
  unsigned char oauth;
};

void initArguments(struct arguments* arguments);

extern struct argp argp;

#define MYTOKEN_USAGE_SET(X) (X->mytoken_issuer.useIt || X->mytoken_profile)

#endif  // OIDC_GEN_OPTIONS_H
