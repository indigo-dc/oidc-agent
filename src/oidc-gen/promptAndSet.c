#include "promptAndSet.h"
#include "account/issuer_helper.h"
#include "defines/agent_values.h"
#include "defines/oidc_values.h"
#include "defines/settings.h"
#include "gen_handler.h"
#include "list/list.h"
#include "utils/file_io/file_io.h"
#include "utils/file_io/oidc_file_io.h"
#include "utils/listUtils.h"
#include "utils/logger.h"
#include "utils/portUtils.h"
#include "utils/prompt.h"

/**
 * @brief prompts the user and sets the account field using the provided
 * function.
 * @param account the account struct that will be updated
 * @param label the string used for prompting
 * @param init the initial (default) value
 * @param set_callback the callback function for setting the account field
 * @param passPrompt indicates if if prompting for a password. If 0 non password
 * prompt is used.
 * @param optional indicating if the field is optional or not.
 */
void promptAndSet(struct oidc_account* account, char* label, const char* init,
                  void (*set_callback)(struct oidc_account*, char*),
                  int passPrompt, int optional) {
  char*     input    = NULL;
  promptFnc prompter = prompt;
  if (passPrompt) {
    prompter = promptPassword;
  }
  do {
    char* text = oidc_sprintf("Please enter %s:", label);
    input      = prompter(text, label, init, CLI_PROMPT_NOT_VERBOSE);
    secFree(text);
    if (strValid(input)) {
      set_callback(account, input);
      break;
    }
    secFree(input);
    if (optional) {
      break;
    }
  } while (1);
}

void promptAndSetMultipleSpaceSeparated(
    struct oidc_account* account, char* label, const char* init_str,
    void (*set_callback)(struct oidc_account*, char*), int optional) {
  list_t* init  = delimitedStringToList(init_str, ' ');
  list_t* input = NULL;
  do {
    char* text = oidc_sprintf("Please enter %s:", label);
    input      = promptMultiple(text, label, init, CLI_PROMPT_NOT_VERBOSE);
    secFree(text);
    if (listValid(input)) {
      char* output = listToDelimitedString(input, " ");
      logger(DEBUG,
             "In %s produced output '%s' for label '%s' with %lu elements",
             __func__, output, label, input->len);
      secFreeList(input);
      set_callback(account, output);
      break;
    }
    secFreeList(input);
    if (optional) {
      break;
    }
  } while (1);
  secFreeList(init);
}

void promptAndSetClientId(struct oidc_account* account) {
  promptAndSet(account, "Client_id", account_getClientId(account),
               account_setClientId, 0, 0);
}

void promptAndSetClientSecret(struct oidc_account* account, int usePubclient) {
  promptAndSet(account, "Client_secret", account_getClientSecret(account),
               account_setClientSecret, 1, usePubclient);
}

void promptAndSetScope(struct oidc_account* account) {
  // TODO if --pub set, should use the max of this public client
  char* supportedScope =
      compIssuerUrls(account_getIssuerUrl(account), ELIXIR_ISSUER_URL)
          ? oidc_strcopy(ELIXIR_SUPPORTED_SCOPES)
          : gen_handleScopeLookup(account_getIssuerUrl(account),
                                  account_getCertPath(account));
  printNormal("This issuer supports the following scopes: %s\n",
              supportedScope);
  if (!strValid(account_getScope(account))) {
    account_setScope(account, oidc_strcopy(DEFAULT_SCOPE));
  }
  promptAndSetMultipleSpaceSeparated(account, "Scopes or 'max'",
                                     account_getScope(account),
                                     account_setScopeExact, 0);
  if (strequal(account_getScope(account), AGENT_SCOPE_ALL)) {
    account_setScope(account, supportedScope);
  } else {
    secFree(supportedScope);
  }
}

void promptAndSetRefreshToken(struct oidc_account* account,
                              struct optional_arg  refresh_token) {
  if (refresh_token.useIt) {
    if (refresh_token.str) {
      account_setRefreshToken(account, oidc_strcopy(refresh_token.str));
      return;
    }
    promptAndSet(account, "Refresh token", account_getRefreshToken(account),
                 account_setRefreshToken, 0, 0);
  }
}

void promptAndSetAudience(struct oidc_account* account,
                          struct optional_arg  audience) {
  if (audience.useIt) {
    if (audience.str) {
      account_setAudience(account, oidc_strcopy(audience.str));
      return;
    }
    promptAndSetMultipleSpaceSeparated(account, "Audiences",
                                       account_getAudience(account),
                                       account_setAudience, 0);
  }
}

void promptAndSetUsername(struct oidc_account* account, list_t* flows) {
  if (findInList(flows, FLOW_VALUE_PASSWORD)) {
    promptAndSet(account, "Username", account_getUsername(account),
                 account_setUsername, 0, 0);
  }
}

void promptAndSetPassword(struct oidc_account* account, list_t* flows) {
  if (findInList(flows, FLOW_VALUE_PASSWORD)) {
    promptAndSet(account, "Password", account_getPassword(account),
                 account_setPassword, 1, 0);
  }
}

void promptAndSetCertPath(struct oidc_account* account,
                          struct optional_arg  cert_path) {
  if (cert_path.useIt && cert_path.str) {
    account_setCertPath(account, oidc_strcopy(cert_path.str));
    return;
  }
  if (!strValid(account_getCertPath(account))) {
    account_setOSDefaultCertPath(account);
  }
  if (cert_path.useIt) {
    promptAndSet(account, "Cert Path", account_getCertPath(account),
                 account_setCertPath, 0, 0);
  }
}

void promptAndSetName(struct oidc_account* account, const char* short_name,
                      const struct optional_arg cnid) {
  char* shortname = oidc_strcopy(short_name);
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

void promptAndSetRedirectUris(struct oidc_account* account, int useDevice) {
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
          useDevice) {
        return;  // redirect_uris only required if no refresh token and no user
                 // credentials provided
      }
    }
  }
}

void _useSuggestedIssuer(struct oidc_account* account) {
  list_t* issuers = getSuggestableIssuers();
  size_t  favPos  = getFavIssuer(account, issuers);
  char*   iss = promptSelect("Please select issuer", "Issuer", issuers, favPos,
                           CLI_PROMPT_NOT_VERBOSE);
  secFreeList(issuers);
  if (!strValid(iss)) {
    printError("Something went wrong. Invalid Issuer.\n");
    exit(EXIT_FAILURE);
  }
  struct oidc_issuer* issuer = secAlloc(sizeof(struct oidc_issuer));
  issuer_setIssuerUrl(issuer, iss);
  account_setIssuer(account, issuer);
}

void promptAndSetIssuer(struct oidc_account* account) {
  if (!oidcFileDoesExist(ISSUER_CONFIG_FILENAME) &&
      !fileDoesExist(ETC_ISSUER_CONFIG_FILE)) {
    promptAndSet(account, "Issuer", account_getIssuerUrl(account),
                 account_setIssuerUrl, 0, 0);
  } else {
    _useSuggestedIssuer(account);
  }
  stringifyIssuerUrl(account);
}
