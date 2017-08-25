#ifndef OIDC_GEN_H
#define OIDC_GEN_H

#include "version.h"
#include <argp.h>

char* possibleCertFiles[] = {
"/etc/ssl/certs/ca-certificates.crt", // Debian/Ubuntu/Gentoo etc.
"/etc/pki/tls/certs/ca-bundle.crt",   // Fedora/RHEL
"/etc/ssl/ca-bundle.pem",             // OpenSUSE
"/etc/pki/tls/cacert.pem"             // OpenELEC
};

#define CONF_ENDPOINT_SUFFIX ".well-known/openid-configuration"

#define OIDC_SOCK_ENV_NAME "OIDC_SOCK"

#define PROVIDER_CONFIG_FILENAME "issuer.config"


const char *argp_program_version = GEN_VERSION;

const char *argp_program_bug_address = BUG_ADDRESS;

struct arguments {
  char* args[1];            /* provider */
  int delete;
  int debug;
  int verbose;
  char* file;
  int registering;
  char* output;
};

static struct argp_option options[] = {
  {"delete", 'd', 0, 0, "delete configuration for the given provider", 0},
  {"debug", 'g', 0, 0, "sets the log level to DEBUG", 0},
  {"verbose", 'v', 0, 0, "enables verbose mode. The stored data will be printed.", 0},
  {"file", 'f', "FILE", 0, "specifies file with client config", 0},
  {"register", 'r', 0, 0, "a new client will be registered", 0},
  {"output", 'o', "OUTPUT_FILE", 0, "the path where the client config will be saved", 0},
  {0}
};

struct oidc_provider* genNewProvider();
char* encryptProvider(const char* json, const char* password) ;
void saveExit(int exitno);
void registerClient(int sock, char* short_name, struct arguments arguments) ;

#endif // OIDC_GEN_H
