#ifndef OIDC_GEN_H
#define OIDC_GEN_H

#include "version.h"
#include "oidc_error.h"
#include <argp.h>

char* possibleCertFiles[] = {
  "/etc/ssl/certs/ca-certificates.crt", // Debian/Ubuntu/Gentoo etc.
  "/etc/pki/tls/certs/ca-bundle.crt",   // Fedora/RHEL
  "/etc/ssl/ca-bundle.pem",             // OpenSUSE
  "/etc/pki/tls/cacert.pem"             // OpenELEC
};

#define CONF_ENDPOINT_SUFFIX ".well-known/openid-configuration"

#define OIDC_SOCK_ENV_NAME "OIDC_SOCK"

#define ACCOUNT_CONFIG_FILENAME "issuer.config"

#define MAX_PASS_TRIES 3


const char *argp_program_version = GEN_VERSION;

const char *argp_program_bug_address = BUG_ADDRESS;

struct arguments {
  char* args[1];            /* account */
  int delete;
  int debug;
  int verbose;
  char* file;
  int manual;
  char* output;
};

static struct argp_option options[] = {
  {"delete", 'd', 0, 0, "delete configuration for the given account", 0},
  {"debug", 'g', 0, 0, "sets the log level to DEBUG", 0},
  {"verbose", 'v', 0, 0, "enables verbose mode. The stored data will be printed.", 0},
  {"file", 'f', "FILE", 0, "specifies file with client config. Implicitly sets -m", 0},
  {"manual", 'm', 0, 0, "Does not use Dynamic Client Registration", 0},
  {"output", 'o', "OUTPUT_FILE", 0, "the path where the client config will be saved", 0},
  {0}
};

struct oidc_account* genNewAccount();
char* encryptAccount(const char* json, const char* password) ;
void saveExit(int exitno);
char* getEncryptionPassword(const char* suggestedPassword, unsigned int max_pass_tries) ;
oidc_error_t encryptAndWriteConfig(const char* text, const char* suggestedPassword, const char* filepath, const char* oidc_filename) ;
void handleDelete(char* short_name) ;
void deleteClient(char* short_name, char* account_json, int revoke) ;
void registerClient(int sock, char* short_name, struct arguments arguments) ;

#endif // OIDC_GEN_H
