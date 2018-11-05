#ifndef AGENT_HANDLER_H
#define AGENT_HANDLER_H

#include "account/account.h"

void agent_handleGen(int sock, list_t* loaded_accounts, char* account_json,
                     const char* flow);
void agent_handleAdd(int sock, list_t* loaded_accounts,
                     const char* account_json, const char* timeout_str);
void agent_handleRm(int sock, list_t* loaded_accounts, char* account_json,
                    int revoke);
void agent_handleToken(int sock, list_t* loaded_accounts, char* short_name,
                       char* min_valid_period_str, const char* scope,
                       const char* application_hint);
void agent_handleRegister(int sock, list_t* loaded_accounts, char* account_json,
                          const char* access_token);
void agent_handleCodeExchange(int sock, list_t* loaded_accounts,
                              char* account_json, char* code,
                              char* redirect_uri, char* state);
void agent_handleStateLookUp(int sock, list_t* loaded_accounts, char* state);
void agent_handleDeviceLookup(int sock, list_t* loaded_accounts,
                              char* account_json, char* device_json);
void agent_handleTermHttp(int sock, char* state);
void agent_handleLock(int sock, char* password, list_t* loaded_accounts,
                      int _lock);

#endif  // AGNET_HANDLER_H
