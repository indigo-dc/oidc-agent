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

// STATUS
#define STATUS_SUCCESS "success"
#define STATUS_FAILURE "failure"
#define STATUS_ACCEPTED "accepted"
#define STATUS_NOTFOUND "NotFound"

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
#define RESPONSE_SUCCESS "{\n\"" IPC_KEY_STATUS "\":\"" STATUS_SUCCESS "\"\n}"
#define RESPONSE_SUCCESS_CLIENT                                          \
  "{\n\"" IPC_KEY_STATUS "\":\"" STATUS_SUCCESS "\",\n\"" IPC_KEY_CLIENT \
  "\":%s\n}"
#define RESPONSE_SUCCESS_INFO                                          \
  "{\n\"" IPC_KEY_STATUS "\":\"" STATUS_SUCCESS "\",\n\"" IPC_KEY_INFO \
  "\":\"%s\"\n}"
#define RESPONSE_ERROR_CLIENT                                            \
  "{\n\"" IPC_KEY_STATUS "\":\"" STATUS_FAILURE "\",\n\"" OIDC_KEY_ERROR \
  "\":\"%s\",\n\"" IPC_KEY_CLIENT "\":%s\n}"
#define RESPONSE_ERROR_CLIENT_INFO                                       \
  "{\n\"" IPC_KEY_STATUS "\":\"" STATUS_FAILURE "\",\n\"" OIDC_KEY_ERROR \
  "\":\"%s\",\n\"" IPC_KEY_CLIENT "\":%s,\n\"" IPC_KEY_INFO "\":\"%s\"\n}"
#define RESPONSE_STATUS_SUCCESS \
  "{\n\"" IPC_KEY_STATUS "\":\"" STATUS_SUCCESS "\"\n}"
#define RESPONSE_STATUS_CONFIG \
  "{\n\"" IPC_KEY_STATUS "\":\"%s\",\n\"" IPC_KEY_CONFIG "\":%s\n}"
#define RESPONSE_STATUS_ACCESS                                 \
  "{\n\"" IPC_KEY_STATUS "\":\"%s\",\n\"" OIDC_KEY_ACCESSTOKEN \
  "\":\"%s\",\n\"" OIDC_KEY_ISSUER "\":\"%s\","                \
  "\n\"" AGENT_KEY_EXPIRESAT "\":%lu\n}"
#define RESPONSE_STATUS_REGISTER \
  "{\n\"" IPC_KEY_STATUS "\":\"%s\",\n\"response\":%s\n}"
#define RESPONSE_STATUS_CODEURI                       \
  "{\n\"" IPC_KEY_STATUS "\":\"%s\",\n\"" IPC_KEY_URI \
  "\":\"%s\",\n\"" OIDC_KEY_STATE "\":\"%s\"\n}"
#define RESPONSE_STATUS_CODEURI_INFO                                     \
  "{\n\"" IPC_KEY_STATUS "\":\"%s\",\n\"" IPC_KEY_URI                    \
  "\":\"%s\",\n\"" OIDC_KEY_STATE "\":\"%s\",\n\"" IPC_KEY_INFO "\":\"%" \
  "s\"\n}"
#define RESPONSE_ERROR                                                   \
  "{\n\"" IPC_KEY_STATUS "\":\"" STATUS_FAILURE "\",\n\"" OIDC_KEY_ERROR \
  "\":\"%s\"\n}"
#define RESPONSE_ERROR_INFO                                              \
  "{\n\"" IPC_KEY_STATUS "\":\"" STATUS_FAILURE "\",\n\"" OIDC_KEY_ERROR \
  "\":\"%s\",\n\"" IPC_KEY_INFO "\":\"%"                                 \
  "s\"\n}"
#define RESPONSE_BADREQUEST                                              \
  "{\n\"" IPC_KEY_STATUS "\":\"" STATUS_FAILURE "\",\n\"" OIDC_KEY_ERROR \
  "\":\"Bad Request: %s\"\n}"
#define RESPONSE_STATUS_INFO \
  "{\n\"" IPC_KEY_STATUS "\":\"%s\",\n\"" IPC_KEY_INFO "\":\"%s\"\n}"
#define RESPONSE_ACCEPTED_DEVICE                                          \
  "{\n\"" IPC_KEY_STATUS "\":\"" STATUS_ACCEPTED "\",\n\"" IPC_KEY_DEVICE \
  "\":%s,\n\"" IPC_KEY_CONFIG "\":%s\n}"

// REQUEST TEMPLATES
#define REQUEST "{\n\"" IPC_KEY_REQUEST "\":\"%s\",\n%s\n}"
#define REQUEST_CONFIG \
  "{\n\"" IPC_KEY_REQUEST "\":\"%s\",\n\"" IPC_KEY_CONFIG "\":%s\n}"
#define REQUEST_ADD_LIFETIME                                                 \
  "{\n\"" IPC_KEY_REQUEST "\":\"" REQUEST_VALUE_ADD "\",\n\"" IPC_KEY_CONFIG \
  "\":%s,\n\"" IPC_KEY_LIFETIME "\":%lu,\n\"" IPC_KEY_PASSWORDENTRY          \
  "\":%s,\n\"" IPC_KEY_CONFIRM "\":%d\n}"
#define REQUEST_ADD                                                          \
  "{\n\"" IPC_KEY_REQUEST "\":\"" REQUEST_VALUE_ADD "\",\n\"" IPC_KEY_CONFIG \
  "\":%s,\n\"" IPC_KEY_PASSWORDENTRY "\":%s,\n\"" IPC_KEY_CONFIRM "\":%d\n}"
#define REQUEST_REMOVE                                 \
  "{\n\"" IPC_KEY_REQUEST "\":\"" REQUEST_VALUE_REMOVE \
  "\",\n\"" IPC_KEY_SHORTNAME "\":\"%s\"\n}"
#define REQUEST_REMOVEALL \
  "{\n\"" IPC_KEY_REQUEST "\":\"" REQUEST_VALUE_REMOVEALL "\"\n}"
#define REQUEST_DELETE                                 \
  "{\n\"" IPC_KEY_REQUEST "\":\"" REQUEST_VALUE_DELETE \
  "\",\n\"" IPC_KEY_CONFIG "\":%s\n}"
#define REQUEST_CONFIG_FLOW                               \
  "{\n\"" IPC_KEY_REQUEST "\":\"%s\",\n\"" IPC_KEY_CONFIG \
  "\":%s,\n\"" IPC_KEY_FLOW "\":%s,\n\"" IPC_KEY_PASSWORDENTRY "\":%s\n}"
#define REQUEST_REGISTER                                 \
  "{\n\"" IPC_KEY_REQUEST "\":\"" REQUEST_VALUE_REGISTER \
  "\",\n\"" IPC_KEY_CONFIG "\":%s,\n\"" IPC_KEY_FLOW "\":%s\n}"
#define REQUEST_REGISTER_AUTH                            \
  "{\n\"" IPC_KEY_REQUEST "\":\"" REQUEST_VALUE_REGISTER \
  "\",\n\"" IPC_KEY_CONFIG "\":%s,\n\"" IPC_KEY_FLOW     \
  "\":%s,\n\"" IPC_KEY_AUTHORIZATION "\":\"%s\"\n}"
#define REQUEST_CODEEXCHANGE                                 \
  "{\n\"" IPC_KEY_REQUEST "\":\"" REQUEST_VALUE_CODEEXCHANGE \
  "\",\n\"" IPC_KEY_REDIRECTEDURI "\":\"%s\"\n}"
#define REQUEST_STATELOOKUP                                 \
  "{\n\"" IPC_KEY_REQUEST "\":\"" REQUEST_VALUE_STATELOOKUP \
  "\",\n\"" OIDC_KEY_STATE "\":\"%s\"\n}"
#define REQUEST_DEVICE                                       \
  "{\n\"" IPC_KEY_REQUEST "\":\"" REQUEST_VALUE_DEVICELOOKUP \
  "\",\n\"" IPC_KEY_DEVICE "\":%s,\n\"" IPC_KEY_CONFIG "\":%s\n}"
#define REQUEST_TERMHTTP                                 \
  "{\n\"" IPC_KEY_REQUEST "\":\"" REQUEST_VALUE_TERMHTTP \
  "\",\n\"" OIDC_KEY_STATE "\":\"%s\"\n}"
#define REQUEST_LOCK \
  "{\n\"" IPC_KEY_REQUEST "\":\"%s\",\n\"" IPC_KEY_PASSWORD "\":\"%s\"\n}"
#define REQUEST_CHECK \
  "{\n\"" IPC_KEY_REQUEST "\":\"" REQUEST_VALUE_CHECK "\"\n}"

#define ACCOUNT_NOT_LOADED "account not loaded"

// internal communication (between oidcp and oidcd)
#define INT_REQUEST_VALUE_UPD_REFRESH "update_refresh"
#define INT_REQUEST_VALUE_AUTOLOAD "autoload"
#define INT_REQUEST_VALUE_CONFIRM "confirm"

#define INT_IPC_KEY_OIDCERRNO "oidc_errno"

#define INT_REQUEST_UPD_REFRESH                                      \
  "{\n\"" IPC_KEY_REQUEST "\":\"" INT_REQUEST_VALUE_UPD_REFRESH      \
  "\",\n\"" IPC_KEY_SHORTNAME "\":\"%s\",\n\"" OIDC_KEY_REFRESHTOKEN \
  "\":\"%s\"\n}"
#define INT_REQUEST_AUTOLOAD                                           \
  "{\n\"" IPC_KEY_REQUEST "\":\"" INT_REQUEST_VALUE_AUTOLOAD           \
  "\",\n\"" IPC_KEY_SHORTNAME "\":\"%s\",\n\"" IPC_KEY_APPLICATIONHINT \
  "\":\"%s\"\n}"
#define INT_REQUEST_CONFIRM                                            \
  "{\n\"" IPC_KEY_REQUEST "\":\"" INT_REQUEST_VALUE_CONFIRM            \
  "\",\n\"" IPC_KEY_SHORTNAME "\":\"%s\",\n\"" IPC_KEY_APPLICATIONHINT \
  "\":\"%s\"\n}"
#define INT_RESPONSE_ERROR                      \
  "{\n\"" IPC_KEY_STATUS "\":\"" STATUS_FAILURE \
  "\",\n\"" INT_IPC_KEY_OIDCERRNO "\":%d\n}"

#endif  // IPC_VALUES_H
