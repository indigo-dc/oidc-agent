#ifndef GEN_HANDLER_H
#define GEN_HANDLER_H

#include "account/account.h"
#include "list/list.h"
#include "oidc-gen_options.h"
#include "utils/oidc_error.h"

void manualGen(struct oidc_account* account, const struct arguments* arguments);
void reauthenticate(const char* shortname, const struct arguments* arguments);
void handleGen(struct oidc_account* account, const struct arguments* arguments,
               const char* cryptPass);
struct oidc_account* manual_genNewAccount(struct oidc_account*    account,
                                          const struct arguments* arguments,
                                          char**                  cryptPassPtr);
struct oidc_account* registerClient(struct arguments* arguments);
void                 handleDelete(const struct arguments*);
oidc_error_t gen_saveAccountConfig(const char* config, const char* shortname,
                                   const char*             hint,
                                   const char*             suggestedPassword,
                                   const struct arguments* arguments);

void  handleCodeExchange(const struct arguments* arguments);
void  stateLookUpWithConfigSave(const char*             state,
                                const struct arguments* arguments);
char* configFromStateLookUp(const char*             state,
                            const struct arguments* arguments);
void  gen_handlePrint(const char* file, const struct arguments* arguments);
char* gen_handleDeviceFlow(char* json_device, char* json_account,
                           const struct arguments* arguments);
oidc_error_t gen_handlePublicClient(struct oidc_account* account,
                                    struct arguments*    arguments);
void         gen_handleList();
void         gen_handleUpdateConfigFile(const char*             file,
                                        const struct arguments* arguments);
char* gen_handleScopeLookup(const char* issuer_url, const char* cert_path);
void gen_handleRename(const char* shortname, const struct arguments* arguments);

void  removeFileFromAgent(const char* filename);
void  writeFileToAgent(const char* filename, const char* data);
char* readFileFromAgent(const char* filename, int ignoreError);
void  handleOnlyAT(struct arguments* arguments);

#endif  // GEN_HANDLER_H
