#ifndef IPC_VALUES_H
#define IPC_VALUES_H

#include "oidc_values.h"

// IPC KEYS
#define IPC_KEY_REQUEST "request"
#define IPC_KEY_STATUS "status"
#define IPC_KEY_CONFIG "config"
#define IPC_KEY_CLIENT "client"
#define IPC_KEY_INFO "info"
#define IPC_KEY_URI "uri"
#define IPC_KEY_SHORTNAME "account"
#define IPC_KEY_PASSWORD "password"
#define IPC_KEY_AUTHORIZATION "authorization"
#define IPC_KEY_DEVICE "oidc_device"
#define IPC_KEY_LIFETIME "lifetime"
#define IPC_KEY_FLOW "flow"
#define IPC_KEY_APPLICATIONHINT "application_hint"
#define IPC_KEY_MINVALID "min_valid_period"
#define IPC_KEY_PASSWORDENTRY "pw_entry"
#define IPC_KEY_CONFIRM "confirm"
#define IPC_KEY_REDIRECTEDURI "redirect_uri"
#define IPC_KEY_FROMGEN "from_gen"
#define IPC_KEY_USECUSTOMSCHEMEURL "prioritizeCustomScheme"
#define IPC_KEY_ISSUERURL "issuer"

// STATUS
#define STATUS_SUCCESS "success"
#define STATUS_FAILURE "failure"
#define STATUS_ACCEPTED "accepted"
#define STATUS_NOTFOUND "NotFound"
#define STATUS_FOUNDBUTDONE "FoundButReceived"

// REQUEST VALUES
#define REQUEST_VALUE_ADD "add"
#define REQUEST_VALUE_GEN "gen"
#define REQUEST_VALUE_REGISTER "register"
#define REQUEST_VALUE_REMOVE "remove"
#define REQUEST_VALUE_REMOVEALL "remove_all"
#define REQUEST_VALUE_DELETE "delete"
#define REQUEST_VALUE_CODEEXCHANGE "code_exchange"
#define REQUEST_VALUE_STATELOOKUP "state_lookup"
#define REQUEST_VALUE_DEVICELOOKUP "device"
#define REQUEST_VALUE_ACCESSTOKEN "access_token"
#define REQUEST_VALUE_TERMHTTP "term_http_server"
#define REQUEST_VALUE_LOCK "lock"
#define REQUEST_VALUE_UNLOCK "unlock"
#define REQUEST_VALUE_CHECK "check"

// RESPONSE TEMPLATES
#define RESPONSE_SUCCESS "{\"" IPC_KEY_STATUS "\":\"" STATUS_SUCCESS "\"}"
#define RESPONSE_SUCCESS_CLIENT \
  "{\"" IPC_KEY_STATUS "\":\"" STATUS_SUCCESS "\",\"" IPC_KEY_CLIENT "\":%s}"
#define RESPONSE_SUCCESS_INFO                                               \
  "{\"" IPC_KEY_STATUS "\":\"" STATUS_SUCCESS "\",\"" IPC_KEY_INFO "\":\"%" \
  "s\"}"
#define RESPONSE_ERROR_CLIENT                                        \
  "{\"" IPC_KEY_STATUS "\":\"" STATUS_FAILURE "\",\"" OIDC_KEY_ERROR \
  "\":\"%s\",\"" IPC_KEY_CLIENT "\":%s}"
#define RESPONSE_ERROR_CLIENT_INFO                                   \
  "{\"" IPC_KEY_STATUS "\":\"" STATUS_FAILURE "\",\"" OIDC_KEY_ERROR \
  "\":\"%s\",\"" IPC_KEY_CLIENT "\":%s,\"" IPC_KEY_INFO "\":\"%s\"}"
#define RESPONSE_STATUS_SUCCESS \
  "{\"" IPC_KEY_STATUS "\":\"" STATUS_SUCCESS "\"}"
#define RESPONSE_STATUS_CONFIG \
  "{\"" IPC_KEY_STATUS "\":\"%s\",\"" IPC_KEY_CONFIG "\":%s}"
#define RESPONSE_STATUS_ACCESS                             \
  "{\"" IPC_KEY_STATUS "\":\"%s\",\"" OIDC_KEY_ACCESSTOKEN \
  "\":\"%s\",\"" OIDC_KEY_ISSUER "\":\"%s\","              \
  "\"" AGENT_KEY_EXPIRESAT "\":%lu}"
#define RESPONSE_STATUS_REGISTER \
  "{\"" IPC_KEY_STATUS "\":\"%s\",\"response\":%s}"
#define RESPONSE_STATUS_CODEURI                   \
  "{\"" IPC_KEY_STATUS "\":\"%s\",\"" IPC_KEY_URI \
  "\":\"%s\",\"" OIDC_KEY_STATE "\":\"%s\"}"
#define RESPONSE_STATUS_CODEURI_INFO                                 \
  "{\"" IPC_KEY_STATUS "\":\"%s\",\"" IPC_KEY_URI                    \
  "\":\"%s\",\"" OIDC_KEY_STATE "\":\"%s\",\"" IPC_KEY_INFO "\":\"%" \
  "s\"}"
#define RESPONSE_ERROR                                               \
  "{\"" IPC_KEY_STATUS "\":\"" STATUS_FAILURE "\",\"" OIDC_KEY_ERROR \
  "\":\"%s\"}"
#define RESPONSE_ERROR_INFO                                          \
  "{\"" IPC_KEY_STATUS "\":\"" STATUS_FAILURE "\",\"" OIDC_KEY_ERROR \
  "\":\"%s\",\"" IPC_KEY_INFO "\":\"%"                               \
  "s\"}"
#define RESPONSE_BADREQUEST                                          \
  "{\"" IPC_KEY_STATUS "\":\"" STATUS_FAILURE "\",\"" OIDC_KEY_ERROR \
  "\":\"Bad Request: %s\"}"
#define RESPONSE_STATUS_INFO \
  "{\"" IPC_KEY_STATUS "\":\"%s\",\"" IPC_KEY_INFO "\":\"%s\"}"
#define RESPONSE_ACCEPTED_DEVICE                                      \
  "{\"" IPC_KEY_STATUS "\":\"" STATUS_ACCEPTED "\",\"" IPC_KEY_DEVICE \
  "\":%s,\"" IPC_KEY_CONFIG "\":%s}"

// REQUEST TEMPLATES
#define REQUEST "{\"" IPC_KEY_REQUEST "\":\"%s\",%s}"
#define REQUEST_ADD_LIFETIME                                             \
  "{\"" IPC_KEY_REQUEST "\":\"" REQUEST_VALUE_ADD "\",\"" IPC_KEY_CONFIG \
  "\":%s,\"" IPC_KEY_LIFETIME "\":%lu,\"" IPC_KEY_PASSWORDENTRY          \
  "\":%s,\"" IPC_KEY_CONFIRM "\":%d}"
#define REQUEST_ADD                                                      \
  "{\"" IPC_KEY_REQUEST "\":\"" REQUEST_VALUE_ADD "\",\"" IPC_KEY_CONFIG \
  "\":%s,\"" IPC_KEY_PASSWORDENTRY "\":%s,\"" IPC_KEY_CONFIRM "\":%d}"
#define REQUEST_REMOVE                                                         \
  "{\"" IPC_KEY_REQUEST "\":\"" REQUEST_VALUE_REMOVE "\",\"" IPC_KEY_SHORTNAME \
  "\":\"%s\"}"
#define REQUEST_REMOVEALL \
  "{\"" IPC_KEY_REQUEST "\":\"" REQUEST_VALUE_REMOVEALL "\"}"
#define REQUEST_DELETE                                                      \
  "{\"" IPC_KEY_REQUEST "\":\"" REQUEST_VALUE_DELETE "\",\"" IPC_KEY_CONFIG \
  "\":%s}"
#define REQUEST_GEN                                                      \
  "{\"" IPC_KEY_REQUEST "\":\"" REQUEST_VALUE_GEN "\",\"" IPC_KEY_CONFIG \
  "\":%s,\"" IPC_KEY_FLOW "\":%s,\"" IPC_KEY_PASSWORDENTRY               \
  "\":%s,\"" IPC_KEY_USECUSTOMSCHEMEURL "\":%d}"
#define REQUEST_REGISTER                                                      \
  "{\"" IPC_KEY_REQUEST "\":\"" REQUEST_VALUE_REGISTER "\",\"" IPC_KEY_CONFIG \
  "\":%s,\"" IPC_KEY_FLOW "\":%s}"
#define REQUEST_REGISTER_AUTH                                                 \
  "{\"" IPC_KEY_REQUEST "\":\"" REQUEST_VALUE_REGISTER "\",\"" IPC_KEY_CONFIG \
  "\":%s,\"" IPC_KEY_FLOW "\":%s,\"" IPC_KEY_AUTHORIZATION "\":\"%s\"}"
#define REQUEST_CODEEXCHANGE                               \
  "{\"" IPC_KEY_REQUEST "\":\"" REQUEST_VALUE_CODEEXCHANGE \
  "\",\"" IPC_KEY_REDIRECTEDURI "\":\"%s\"}"
#define REQUEST_CODEEXCHANGEGEN                            \
  "{\"" IPC_KEY_REQUEST "\":\"" REQUEST_VALUE_CODEEXCHANGE \
  "\",\"" IPC_KEY_REDIRECTEDURI "\":\"%s\",\"" IPC_KEY_FROMGEN "\":1}"
#define REQUEST_STATELOOKUP                               \
  "{\"" IPC_KEY_REQUEST "\":\"" REQUEST_VALUE_STATELOOKUP \
  "\",\"" OIDC_KEY_STATE "\":\"%s\"}"
#define REQUEST_DEVICE                                     \
  "{\"" IPC_KEY_REQUEST "\":\"" REQUEST_VALUE_DEVICELOOKUP \
  "\",\"" IPC_KEY_DEVICE "\":%s,\"" IPC_KEY_CONFIG "\":%s}"
#define REQUEST_TERMHTTP                                                      \
  "{\"" IPC_KEY_REQUEST "\":\"" REQUEST_VALUE_TERMHTTP "\",\"" OIDC_KEY_STATE \
  "\":\"%s\"}"
#define REQUEST_LOCK \
  "{\"" IPC_KEY_REQUEST "\":\"%s\",\"" IPC_KEY_PASSWORD "\":\"%s\"}"
#define REQUEST_CHECK "{\"" IPC_KEY_REQUEST "\":\"" REQUEST_VALUE_CHECK "\"}"

#define ACCOUNT_NOT_LOADED "account not loaded"

// internal communication (between oidcp and oidcd)
#define INT_REQUEST_VALUE_UPD_REFRESH "update_refresh"
#define INT_REQUEST_VALUE_AUTOLOAD "autoload"
#define INT_REQUEST_VALUE_CONFIRM "confirm"
#define INT_REQUEST_VALUE_QUERY_ACCDEFAULT "query_account_default"

#define INT_IPC_KEY_OIDCERRNO "oidc_errno"

#define INT_REQUEST_UPD_REFRESH                               \
  "{\"" IPC_KEY_REQUEST "\":\"" INT_REQUEST_VALUE_UPD_REFRESH \
  "\",\"" IPC_KEY_SHORTNAME "\":\"%s\",\"" OIDC_KEY_REFRESHTOKEN "\":\"%s\"}"
#define INT_REQUEST_AUTOLOAD                                       \
  "{\"" IPC_KEY_REQUEST "\":\"" INT_REQUEST_VALUE_AUTOLOAD         \
  "\",\"" IPC_KEY_SHORTNAME "\":\"%s\",\"" IPC_KEY_APPLICATIONHINT \
  "\":\"%s\"}"
#define INT_REQUEST_AUTOLOAD_WITH_ISSUER                     \
  "{\"" IPC_KEY_REQUEST "\":\"" INT_REQUEST_VALUE_AUTOLOAD   \
  "\",\"" IPC_KEY_SHORTNAME "\":\"%s\",\"" IPC_KEY_ISSUERURL \
  "\":\"%s\",\"" IPC_KEY_APPLICATIONHINT "\":\"%s\"}"
#define INT_REQUEST_CONFIRM                                        \
  "{\"" IPC_KEY_REQUEST "\":\"" INT_REQUEST_VALUE_CONFIRM          \
  "\",\"" IPC_KEY_SHORTNAME "\":\"%s\",\"" IPC_KEY_APPLICATIONHINT \
  "\":\"%s\"}"
#define INT_REQUEST_CONFIRM_WITH_ISSUER                      \
  "{\"" IPC_KEY_REQUEST "\":\"" INT_REQUEST_VALUE_CONFIRM    \
  "\",\"" IPC_KEY_ISSUERURL "\":\"%s\",\"" IPC_KEY_SHORTNAME \
  "\":\"%s\",\"" IPC_KEY_APPLICATIONHINT "\":\"%s\"}"
#define INT_REQUEST_QUERY_ACCDEFAULT_ISSUER                        \
  "{\"" IPC_KEY_REQUEST "\":\"" INT_REQUEST_VALUE_QUERY_ACCDEFAULT \
  "\",\"" IPC_KEY_ISSUERURL "\":\"%s\"}"
#define INT_REQUEST_QUERY_ACCDEFAULT \
  "{\"" IPC_KEY_REQUEST "\":\"" INT_REQUEST_VALUE_QUERY_ACCDEFAULT "\"}"
#define INT_RESPONSE_ACCDEFAULT                                         \
  "{\"" IPC_KEY_STATUS "\":\"" STATUS_SUCCESS "\",\"" IPC_KEY_SHORTNAME \
  "\":\"%s\"}"
#define INT_RESPONSE_ERROR                                                  \
  "{\"" IPC_KEY_STATUS "\":\"" STATUS_FAILURE "\",\"" INT_IPC_KEY_OIDCERRNO \
  "\":%d}"

#endif  // IPC_VALUES_H
