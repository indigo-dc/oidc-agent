#ifndef OIDCD_HANDLER_H
#define OIDCD_HANDLER_H

#include "account/account.h"
#include "ipc/pipe.h"
#include "oidc-agent/oidc-agent_options.h"

void oidcd_handleGen(struct ipcPipe pipes, const char* account_json,
                     const char* flow, const char* nowebserver_str,
                     const char*             noscheme_str,
                     const struct arguments* arguments);
void oidcd_handleAdd(struct ipcPipe, const char* account_json,
                     const char* timeout_str, const char* confirm_str);
void oidcd_handleDelete(struct ipcPipe, const char* account_json);
void oidcd_handleRm(struct ipcPipe, char* account_name);
void oidcd_handleRemoveAll(struct ipcPipe);
void oidcd_handleToken(struct ipcPipe, char* short_name,
                       const char* min_valid_period_str, const char* scope,
                       const char* application_hint, const struct arguments*);
void oidcd_handleTokenIssuer(struct ipcPipe pipes, char* issuer,
                             const char* min_valid_period_str,
                             const char* scope, const char* application_hint,
                             const struct arguments* arguments);
void oidcd_handleRegister(struct ipcPipe, const char* account_json,
                          const char* json_str, const char* access_token);
void oidcd_handleCodeExchange(struct ipcPipe pipes, const char* redirected_uri,
                              const char* fromString);
void oidcd_handleStateLookUp(struct ipcPipe, char* state);
void oidcd_handleDeviceLookup(struct ipcPipe, const char* account_json,
                              const char* device_json);
void oidcd_handleTermHttp(struct ipcPipe, const char* state);
void oidcd_handleLock(struct ipcPipe, const char* password, int _lock);

#endif  // OIDCD_HANDLER_H
