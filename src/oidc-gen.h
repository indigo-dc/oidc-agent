#ifndef OIDC_GEN_H
#define OIDC_GEN_H

#include "version.h"
#include "oidc_error.h"
#include <argp.h>

#include "account.h"

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
  int codeFlow;
  char* codeExchangeRequest;
};

/* Keys for options without short-options. */
#define OPT_codeExchangeRequest 1

static struct argp_option options[] = {
  {"delete", 'd', 0, 0, "delete configuration for the given account", 0},
  {"debug", 'g', 0, 0, "sets the log level to DEBUG", 0},
  {"verbose", 'v', 0, 0, "enables verbose mode. The stored data will be printed.", 0},
  {"file", 'f', "FILE", 0, "specifies file with client config. Implicitly sets -m", 0},
  {"manual", 'm', 0, 0, "Does not use Dynamic Client Registration", 0},
  {"output", 'o', "OUTPUT_FILE", 0, "the path where the client config will be saved", 0},
  {"codeExchangeRequest", OPT_codeExchangeRequest, "REQUEST", OPTION_HIDDEN, "The code Exchange REQUEST", 0},
  {"codeFlow", 'c', 0, 0, "uses the Authorization Code Flow instead of Password Flow.", 0},
  {0, 0, 0, 0, 0, 0}
};

/**
 * @brief initializes arguments
 * @param arguments the arguments struct
 */
void initArguments(struct arguments* arguments) {
  arguments->delete = 0;
  arguments->debug = 0;
  arguments->args[0] = NULL;
  arguments->file = NULL;
  arguments->manual = 0;
  arguments->verbose = 0;
  arguments->output = NULL;
  arguments->codeFlow = 0;
  arguments->codeExchangeRequest = NULL;
}

void initArguments(struct arguments* arguments) ;
void assertOidcDirExists() ;
void manualGen(struct oidc_account* account, const char* short_name, int verbose, int codeFlow) ;
struct oidc_account* genNewAccount(struct oidc_account* account, const char* short_name, char** cryptPassPtr) ;
void registerClient(char* short_name, const char* output, int verbose) ;
void handleDelete(char* short_name) ;
void deleteClient(char* short_name, char* account_json, int revoke) ;
struct oidc_account* accountFromFile(const char* filename) ;
void updateIssuerConfig(const char* issuer_url) ;
oidc_error_t encryptAndWriteConfig(const char* text, const char* suggestedPassword, const char* filepath, const char* oidc_filename) ;
void promptAndSet(struct oidc_account* account, char* prompt_str, void (*set_callback)(struct oidc_account*, char*), char* (*get_callback)(struct oidc_account), int passPrompt, int optional) ;
void promptAndSetIssuer(struct oidc_account* account) ;
void promptAndSetClientId(struct oidc_account* account) ;
void promptAndSetClientSecret(struct oidc_account* account) ;
void promptAndSetRefreshToken(struct oidc_account* account) ;
void promptAndSetUsername(struct oidc_account* account) ;
void promptAndSetPassword(struct oidc_account* account) ;
void promptAndSetCertPath(struct oidc_account* account) ;
void promptAndSetName(struct oidc_account* account, const char* short_name) ;
void useSuggestedIssuer(struct oidc_account* account) ;
void promptAndSetRedirectUris(struct oidc_account* account) ;
int promptIssuer(struct oidc_account* account, const char* fav) ;
void stringifyIssuerUrl(struct oidc_account* account) ;
char* encryptAccount(const char* json, const char* password) ;
char* getEncryptionPassword(const char* suggestedPassword, unsigned int max_pass_tries) ;
char* createClientConfigFileName(const char* issuer_url, const char* client_id) ;
void handleCodeExchange(char* request, char* short_name, int verbose) ;

#endif // OIDC_GEN_H
