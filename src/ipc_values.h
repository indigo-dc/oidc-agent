#ifndef IPC_VALUES_H
#define IPC_VALUES_H

//RESPONSE TEMPLATES
#define RESPONSE_SUCCESS_CLIENT "{\n\"status\":\"success\",\n\"client\":%s\n}"
#define RESPONSE_ERROR_CLIENT_INFO "{\n\"status\":\"failure\",\n\"error\":\"%s\",\n\"client\":%s,\n\"info\":\"%s\"\n}"
#define RESPONSE_STATUS_SUCCESS "{\n\"status\":\"success\"\n}"
#define RESPONSE_STATUS_CONFIG "{\n\"status\":\"%s\",\n\"config\":%s\n}"
#define RESPONSE_STATUS_ACCESS "{\n\"status\":\"%s\",\n\"access_token\":\"%s\"\n}"
#define RESPONSE_STATUS_ACCOUNT "{\n\"status\":\"%s\",\n\"account_list\":%s\n}"
#define RESPONSE_STATUS_REGISTER "{\n\"status\":\"%s\",\n\"response\":%s\n}"
#define RESPONSE_STATUS_CODEURI "{\n\"status\":\"%s\",\n\"uri\":\"%s\",\n\"state\":\"%s\"\n}"
#define RESPONSE_STATUS_CODEURI_INFO "{\n\"status\":\"%s\",\n\"uri\":\"%s\",\n\"state\":\"%s\",\n\"info\":\"%s\"\n}"
#define RESPONSE_ERROR "{\n\"status\":\"failure\",\n\"error\":\"%s\"\n}"
#define RESPONSE_BADREQUEST "{\n\"status\":\"failure\",\n\"error\":\"Bad Request: %s\"\n}"
#define RESPONSE_STATUS_INFO "{\n\"status\":\"%s\",\n\"info\":\"%s\"\n}"

//REQUEST TEMPLATES
#define REQUEST "{\n\"request\":\"%s\",\n%s\n}"
#define REQUEST_CONFIG "{\n\"request\":\"%s\",\n\"config\":%s\n}"
#define REQUEST_CONFIG_AUTH "{\n\"request\":\"%s\",\n\"config\":%s,\n\"authorization\":\"%s\"\n}"
#define REQUEST_CONFIG_FLOW "{\n\"request\":\"%s\",\n\"config\":%s,\n\"flow\":%s\n}"
#define REQUEST_CODEEXCHANGE "{\n\"request\":\"code_exchange\",\n\"config\":%s,\n\"redirect_uri\":\"%s\",\n\"code\":\"%s\",\n\"state\":\"%s\"\n}"
#define REQUEST_STATELOOKUP "{\n\"request\":\"state_lookup\",\n\"state\":\"%s\"\n}"

//STATUS
#define STATUS_SUCCESS "success"
#define STATUS_FAILURE "failure"
#define STATUS_ACCEPTED "accepted"
#define STATUS_NOTFOUND "NotFound"

//REQUEST VALUES
#define REQUEST_VALUE_ADD "add"
#define REQUEST_VALUE_GEN "gen"
#define REQUEST_VALUE_REGISTER "register"
#define REQUEST_VALUE_REMOVE "remove"
#define REQUEST_VALUE_DELETE "delete"
#define REQUEST_VALUE_CODEEXCHANGE "code_exchange"
#define REQUEST_VALUE_STATELOOKUP "state_lookup"
#define REQUEST_VALUE_ACCESSTOKEN "access_token"
#define REQUEST_VALUE_ACCOUNTLIST "account_list"

//FLOW VALUES
#define FLOW_VALUE_CODE "code"
#define FLOW_VALUE_PASSWORD "password"
#define FLOW_VALUE_DEVICE "device"
#define FLOW_VALUE_REFRESH "refresh"

#define ACCOUNT_NOT_LOADED "account not loaded"


#endif //IPC_VALUES_H
