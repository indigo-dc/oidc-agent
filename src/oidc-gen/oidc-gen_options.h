#ifndef OIDC_GEN_OPTIONS_H
#define OIDC_GEN_OPTIONS_H

#include "list/list.h"

#include <argp.h>

struct optional_arg {
  char* str;
  short useIt;
};

struct arguments {
  char* args[1]; /* account */
  char* file;
  char* output;
  char* codeExchange;
  char* state;
  char* print;
  char* device_authorization_endpoint;
  char* updateConfigFile;
  char* pw_cmd;
  char* rename;

  struct optional_arg dynRegToken;
  struct optional_arg cert_path;
  struct optional_arg refresh_token;
  struct optional_arg cnid;
  struct optional_arg audience;

  list_t* flows;
  list_t* redirect_uris;

  unsigned char delete;
  unsigned char debug;
  unsigned char verbose;
  unsigned char manual;
  unsigned char listClients;
  unsigned char listAccounts;
  unsigned char qr;
  unsigned char qrterminal;
  unsigned char splitConfigFiles;
  unsigned char seccomp;
  unsigned char _nosec;
  unsigned char noUrlCall;
  unsigned char usePublicClient;
  unsigned char noWebserver;
  unsigned char noScheme;
  unsigned char reauthenticate;
};

void initArguments(struct arguments* arguments);

struct argp argp;

#endif  // OIDC_GEN_OPTIONS_H
