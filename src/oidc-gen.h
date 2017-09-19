#ifndef OIDC_GEN_H
#define OIDC_GEN_H

#include "version.h"
#include "oidc_error.h"
#include <argp.h>

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
char* getEncryptionPassword(const char* suggestedPassword, unsigned int max_pass_tries) ;
oidc_error_t encryptAndWriteConfig(const char* text, const char* suggestedPassword, const char* filepath, const char* oidc_filename) ;
void handleDelete(char* short_name) ;
void deleteClient(char* short_name, char* account_json, int revoke) ;
void registerClient(char* short_name, const char* output, int verbose) ;
void promptAndSet(struct oidc_account* account, char* prompt_str, void (*set_callback)(struct oidc_account*, char*), char* (*get_callback)(struct oidc_account), int passPrompt, int optional) ;
void manualGen(struct oidc_account* account, const char* short_name, int verbose) ;

#endif // OIDC_GEN_H
