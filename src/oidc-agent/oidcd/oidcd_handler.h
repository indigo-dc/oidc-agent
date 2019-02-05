#ifndef OIDCD_HANDLER_H
#define OIDCD_HANDLER_H

#include "account/account.h"
#include "ipc/pipe.h"

void oidcd_handleGen(struct ipcPipe, list_t*, const char* account_json,
                     const char* flow);
void oidcd_handleAdd(struct ipcPipe, list_t*, const char* account_json,
                     const char* timeout_str);
void oidcd_handleDelete(struct ipcPipe, list_t*, const char* account_json);
void oidcd_handleRm(struct ipcPipe, list_t*, char* account_name);
void oidcd_handleRemoveAll(struct ipcPipe, list_t**);
void oidcd_handleToken(struct ipcPipe, list_t*, char* short_name,
                       const char* min_valid_period_str, const char* scope,
                       const char* application_hint);
void oidcd_handleRegister(struct ipcPipe, list_t*, const char* account_json,
                          const char* json_str, const char* access_token);
void oidcd_handleCodeExchange(struct ipcPipe, list_t*, const char* account_json,
                              const char* code, const char* redirect_uri,
                              const char* state, char* code_verifier);
void oidcd_handleStateLookUp(struct ipcPipe, list_t*, char* state);
void oidcd_handleDeviceLookup(struct ipcPipe, list_t*, const char* account_json,
                              const char* device_json);
void oidcd_handleTermHttp(struct ipcPipe, const char* state);
void oidcd_handleLock(struct ipcPipe, const char* password, list_t*, int _lock);

#endif  // OIDCD_HANDLER_H
