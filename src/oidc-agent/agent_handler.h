#ifndef AGENT_HANDLER_H
#define AGENT_HANDLER_H

#include "account/account.h"
#include "ipc/pipe.h"

void agent_handleGen(struct ipcPipe, list_t*, const char* account_json,
                     const char* flow);
void agent_handleAdd(struct ipcPipe, list_t*, const char* account_json,
                     const char* timeout_str);
void agent_handleDelete(struct ipcPipe, list_t*, const char* account_json);
void agent_handleRm(struct ipcPipe, list_t*, char* account_name);
void agent_handleRemoveAll(struct ipcPipe, list_t**);
void agent_handleToken(struct ipcPipe, list_t*, char* short_name,
                       const char* min_valid_period_str, const char* scope,
                       const char* application_hint);
void agent_handleRegister(struct ipcPipe, list_t*, const char* account_json,
                          const char* json_str, const char* access_token);
void agent_handleCodeExchange(struct ipcPipe, list_t*, const char* account_json,
                              const char* code, const char* redirect_uri,
                              const char* state, char* code_verifier);
void agent_handleStateLookUp(struct ipcPipe, list_t*, char* state);
void agent_handleDeviceLookup(struct ipcPipe, list_t*, const char* account_json,
                              const char* device_json);
void agent_handleTermHttp(struct ipcPipe, const char* state);
void agent_handleLock(struct ipcPipe, const char* password, list_t*, int _lock);

#endif  // AGNET_HANDLER_H
