#ifndef OIDCD_HANDLER_H
#define OIDCD_HANDLER_H

#include "account/account.h"
#include "ipc/pipe.h"
#include "oidc-agent/oidc-agent_options.h"

void oidcd_handleGen(struct ipcPipe pipes, const char* account_json,
                     const char* flow, const char* nowebserver_str,
                     const char* noscheme_str, const char* only_at,
                     const struct arguments* arguments);
void oidcd_handleReauthenticate(struct ipcPipe pipes, char* short_name,
                                const struct arguments* arguments);
void oidcd_handleAdd(struct ipcPipe, const char* account_json,
                     const char* timeout_str, const char* confirm_str,
                     const char* alwaysallowid);
void oidcd_handleDelete(struct ipcPipe, const char* account_json);
void oidcd_handleDeleteClient(struct ipcPipe pipes, const char* client_uri,
                              const char* registration_access_token,
                              const char* cert_path);
void oidcd_handleRm(struct ipcPipe, char* account_name);
void oidcd_handleRemoveAll(struct ipcPipe);
void oidcd_handleToken(struct ipcPipe, const char* short_name,
                       const char* min_valid_period_str, const char* scope,
                       const char* application_hint, const char* audience,
                       const struct arguments*);
void oidcd_handleTokenIssuer(struct ipcPipe pipes, const char* issuer,
                             const char* min_valid_period_str,
                             const char* scope, const char* application_hint,
                             const char*             audience,
                             const struct arguments* arguments);
void oidcd_handleIdToken(struct ipcPipe pipes, const char* short_name,
                         const char* issuer, const char* scope,
                         const char*             application_hint,
                         const struct arguments* arguments);
void oidcd_handleMytoken(struct ipcPipe pipes, const char* short_name,
                         const char* profile, const char* application_hint,
                         const struct arguments* arguments);
void oidcd_handleRegister(struct ipcPipe, const char* account_json,
                          const char* json_str, const char* access_token);
void oidcd_handleCodeExchange(struct ipcPipe pipes, const char* redirected_uri,
                              const char* fromString);
void oidcd_handleStateLookUp(struct ipcPipe, char* state);
void oidcd_handleDeviceLookup(struct ipcPipe, const char* device_json,
                              const char* only_at_str);
void oidcd_handleScopes(struct ipcPipe pipes, const char* issuer_url,
                        const char* config_endpoint, const char* cert_path);
void oidcd_handleMytokenProvidersLookup(struct ipcPipe pipes,
                                        const char*    mytoken_url,
                                        const char*    config_endpoint,
                                        const char*    cert_path);
void oidcd_handleListLoadedAccounts(struct ipcPipe pipes);
void oidcd_handleTermHttp(struct ipcPipe, const char* state);
void oidcd_handleLock(struct ipcPipe, const char* password, int _lock);
void oidcd_handleAgentStatus(struct ipcPipe          pipes,
                             const struct arguments* arguments);
void oidcd_handleAgentStatusJSON(struct ipcPipe          pipes,
                                 const struct arguments* arguments);
void oidcd_handleFileRemove(struct ipcPipe pipes, const char* filename);
void oidcd_handleFileRead(struct ipcPipe pipes, const char* filename);
void oidcd_handleFileWrite(struct ipcPipe pipes, const char* filename,
                           const char* data);

char* _oidcd_getMytokenConfirmation(struct ipcPipe pipes,
                                    const char*    base64html);
#endif  // OIDCD_HANDLER_H
