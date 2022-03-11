#include "response.h"

#include "api_helper.h"
#include "memory.h"
#include "utils/oidc_error.h"
#include "utils/printer.h"
#include "utils/string/stringUtils.h"

unsigned char _checkLocalResponseForRemote(struct agent_response res) {
  if (res.type == AGENT_RESPONSE_TYPE_TOKEN &&
      res.token_response.token != NULL) {
    return LOCAL_COMM;
  }
  const char* err =
      res.type == AGENT_RESPONSE_TYPE_ERROR && res.error_response.error != NULL
          ? res.error_response.error
          : oidc_serror();
  if (strequal(err, "No account configured with that short name") ||
      strstarts(err, "Could not connect to oidc-agent") ||
      strequal(err, "OIDC_SOCK env var not set")) {
    return REMOTE_COMM;
  }
  return LOCAL_COMM;
}

void secFreeTokenResponse(struct token_response token_response) {
  START_APILOGLEVEL
  secFree(token_response.token);
  secFree(token_response.issuer);
  END_APILOGLEVEL
}

void secFreeErrorResponse(struct agent_error_response error_response) {
  START_APILOGLEVEL
  secFree(error_response.error);
  secFree(error_response.help);
  END_APILOGLEVEL
}

void secFreeLoadedAccountsListResponse(
    struct loaded_accounts_response response) {
  START_APILOGLEVEL
  secFree(response.accounts);
  END_APILOGLEVEL
}

void secFreeAgentResponse(struct agent_response agent_response) {
  START_APILOGLEVEL
  switch (agent_response.type) {
    case AGENT_RESPONSE_TYPE_ERROR:
      secFreeErrorResponse(agent_response.error_response);
      break;
    case AGENT_RESPONSE_TYPE_TOKEN:
      secFreeTokenResponse(agent_response.token_response);
      break;
    case AGENT_RESPONSE_TYPE_ACCOUNTS:
      secFreeLoadedAccountsListResponse(
          agent_response.loaded_accounts_response);
  }
  END_APILOGLEVEL
}

void oidcagent_printErrorResponse(struct agent_error_response err) {
  if (err.error) {
    printError("Error: %s\n", err.error);
  }
  if (err.help) {
    printImportant("%s", err.help);
  }
}