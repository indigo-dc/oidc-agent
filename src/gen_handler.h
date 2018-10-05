#ifndef GEN_HANDLER_H
#define GEN_HANDLER_H

#include "account.h"
#include "oidc-gen_options.h"

void manualGen(struct oidc_account* account, struct arguments arguments) ;
void handleGen(struct oidc_account* account, struct arguments arguments, char** cryptPassPtr) ;
struct oidc_account* genNewAccount(struct oidc_account* account, struct arguments arguments, char** cryptPassPtr) ;
struct oidc_account* registerClient(struct arguments arguments) ;
void handleDelete(struct arguments) ;
void deleteClient(char* short_name, char* account_json, int revoke) ;
struct oidc_account* accountFromFile(const char* filename) ;
void updateIssuerConfig(const char* issuer_url) ;
oidc_error_t encryptAndWriteConfig(const char* text, const char* hint, const char* suggestedPassword, const char* filepath, const char* oidc_filename) ;
void promptAndSet(struct oidc_account* account, char* prompt_str, void (*set_callback)(struct oidc_account*, char*), char* (*get_callback)(struct oidc_account), int passPrompt, int optional) ;
void promptAndSetIssuer(struct oidc_account* account) ;
void promptAndSetClientId(struct oidc_account* account) ;
void promptAndSetClientSecret(struct oidc_account* account) ;
void promptAndSetRefreshToken(struct oidc_account* account) ;
void promptAndSetUsername(struct oidc_account* account) ;
void promptAndSetPassword(struct oidc_account* account) ;
void promptAndSetCertPath(struct oidc_account* account, struct optional_arg cert_path) ;
void promptAndSetName(struct oidc_account* account, const char* short_name, struct optional_arg client_name_id) ;
void promptAndSetScope(struct oidc_account* account) ;
void useSuggestedIssuer(struct oidc_account* account) ;
void promptAndSetRedirectUris(struct oidc_account* account, int useDevice) ;
int promptIssuer(struct oidc_account* account, const char* fav) ;
void stringifyIssuerUrl(struct oidc_account* account) ;
char* encryptAccount(const char* json, const char* password) ;
char* getEncryptionPassword(const char* forWhat, const char* suggestedPassword, unsigned int max_pass_tries) ;
char* createClientConfigFileName(const char* issuer_url, const char* client_id) ;
void handleCodeExchange(struct arguments arguments) ;
void handleStateLookUp(const char* state, struct arguments arguments) ;
void gen_handleList() ;
void gen_handlePrint(const char* file) ;
char* gen_handleDeviceFlow(char* json_device, char* json_account, struct arguments arguments) ;
void registerSignalHandler(const char* state) ;
void gen_http_signal_handler(int signo) ;
void unregisterSignalHandler() ;

#endif //GEN_HANDLER_H
