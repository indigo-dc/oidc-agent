#include "defines/agent_values.h"
#include "defines/oidc_values.h"
#include "defines/settings.h"
#include "gen_handler.h"
#include "list/list.h"
#include "oidc-gen/oidc-gen_options.h"
#include "utils/listUtils.h"
#include "utils/logger.h"
#include "utils/portUtils.h"

void promptAndSetName(struct oidc_account*    account,
                      const struct arguments* arguments) {
  char* shortname = oidc_strcopy(arguments->args[0]);
  while (!strValid(shortname)) {
    secFree(shortname);
    shortname = prompt("Enter short name for the account to configure",
                       "short name", NULL, CLI_PROMPT_VERBOSE);
  }
  char* client_identifier =
      cnid.useIt
          ? cnid.str
                ? oidc_strcopy(cnid.str)
                : prompt("Enter optional additional client-name-identifier",
                         "client-name-identifier", NULL, CLI_PROMPT_VERBOSE)
          : NULL;
  account_setName(account, shortname, client_identifier);
  secFree(client_identifier);
}

oidc_error_t checkRedirectUrisForErrors(list_t* redirect_uris) {
  if (redirect_uris == NULL || redirect_uris->len == 0) {
    return OIDC_EARGNULL;
  }
  oidc_error_t     err = OIDC_SUCCESS;
  list_node_t*     node;
  list_iterator_t* it = list_iterator_new(redirect_uris, LIST_HEAD);
  while ((node = list_iterator_next(it))) {
    if (strstarts(node->val, AGENT_CUSTOM_SCHEME)) {
      continue;
    }
    unsigned int port = getPortFromUri(node->val);
    if (port == 0) {
      printError("%s is not a valid redirect_uri. The redirect uri has to "
                 "be in the following format: http://localhost:<port>[/*] or "
                 "%s<anything>\n",
                 (char*)node->val, AGENT_CUSTOM_SCHEME);
      err = OIDC_EERROR;
    } else if (port < MIN_PORT || port > MAX_PORT) {
      printError("The port number has to be between %d and %d\n", MIN_PORT,
                 MAX_PORT);
      err = OIDC_EERROR;
    }
  }
  list_iterator_destroy(it);
  return err;
}

void promptAndSetRedirectUris(struct oidc_account*    account,
                              const struct arguments* arguments) {
  list_t* current_uris = account_getRedirectUris(account);
  if (current_uris) {
    oidc_error_t err =
        checkRedirectUrisForErrors(account_getRedirectUris(account));
    if (err == OIDC_EERROR) {
      account_setRedirectUris(account, NULL);
      current_uris = NULL;
    }
  }
  while (1) {
    list_t* redirect_uris =
        promptMultiple("Please enter Redirect URIs", "Redirect_uris",
                       current_uris, CLI_PROMPT_NOT_VERBOSE);
    oidc_error_t err = checkRedirectUrisForErrors(redirect_uris);
    if (err == OIDC_SUCCESS) {
      account_setRedirectUris(account, redirect_uris);
      return;
    }
    secFreeList(redirect_uris);
    if (err !=
        OIDC_EERROR) {  // If err == OIDC_EERROR a not valid redirect_uri
                        // was entered, so the user want to provide input.
                        // If err != OIDC_EERROR no input was provided
      if (account_refreshTokenIsValid(account) ||
          (strValid(account_getUsername(account)) &&
           strValid(account_getPassword(account))) ||
          (arguments->flows &&
           strequal(list_at(arguments->flows, 0)->val, FLOW_VALUE_DEVICE))) {
        return;  // redirect_uris only required if no refresh token and no user
                 // credentials provided
      }
    }
  }
}
