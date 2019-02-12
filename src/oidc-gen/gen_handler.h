#ifndef GEN_HANDLER_H
#define GEN_HANDLER_H

#include "account/account.h"
#include "list/list.h"
#include "oidc-gen_options.h"
#include "utils/oidc_error.h"

void manualGen(struct oidc_account* account, const struct arguments* arguments);
void handleGen(struct oidc_account* account, const struct arguments* arguments,
               char** cryptPassPtr);
struct oidc_account* genNewAccount(struct oidc_account*    account,
                                   const struct arguments* arguments,
                                   char**                  cryptPassPtr);
struct oidc_account* registerClient(struct arguments* arguments);
void                 handleDelete(const struct arguments*);
oidc_error_t encryptAndWriteConfig(const char* config, const char* shortname,
                                   const char* hint,
                                   const char* suggestedPassword,
                                   const char* filepath,
                                   const char* oidc_filename, int verbose);

void  handleCodeExchange(const struct arguments* arguments);
void  handleStateLookUp(const char* state, const struct arguments* arguments);
void  gen_handlePrint(const char* file);
char* gen_handleDeviceFlow(char* json_device, char* json_account,
                           const struct arguments* arguments);
oidc_error_t gen_handlePublicClient(struct oidc_account* account,
                                    struct arguments*    arguments);
void         gen_handleList();
void         gen_handleUpdateConfigFile(const char* shortname);

#endif  // GEN_HANDLER_H
