#ifndef AGENT_HANDLER_H
#define AGENT_HANDLER_H

#include "account.h"

void agent_handleGen(int sock, struct oidc_account** loaded_p, size_t* loaded_p_count, char* account_json, const char* flow) ;
void agent_handleAdd(int sock, struct oidc_account** loaded_p, size_t* loaded_p_count, char* account_json) ;
void agent_handleRm(int sock, struct oidc_account** loaded_p, size_t* loaded_p_count, char* account_json, int revoke) ;
void agent_handleToken(int sock, struct oidc_account* loaded_p, size_t loaded_p_count, char* short_name, char* min_valid_period_str) ;
void agent_handleList(int sock, struct oidc_account* loaded_p, size_t loaded_p_count) ;
void agent_handleRegister(int sock, struct oidc_account* loaded_p, size_t loaded_p_count, char* account_json, const char* access_token) ;
void agent_handleCodeExchange(int sock, struct oidc_account** loaded_p, size_t* loaded_p_count, char* account_json, char* code, char* redirect_uri, char* state) ;
void agent_handleStateLookUp(int sock, struct oidc_account* loaded_p, size_t loaded_p_count, char* state) ;

#endif //AGNET_HANDLER_H
