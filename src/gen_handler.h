#ifndef GEN_HANDLER_H
#define GEN_HANDLER_H

#include "account.h"

void manualGen(struct oidc_account* account, const char* short_name, int verbose, char* flow) ;
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
void handleStateLookUp(char* state) ;

#endif //GEN_HANDLER_H
