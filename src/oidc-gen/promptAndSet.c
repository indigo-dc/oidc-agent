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
#include "utils/portUtils.h"
#include "utils/prompt.h"

/**
 * @brief prompts the user and sets the account field using the provided
 * function.
 * @param account the account struct that will be updated
 * @param prompt_str the string used for prompting
 * @param set_callback the callback function for setting the account field
 * @param get_callback the callback function for getting the account field
 * @param passPrompt indicates if if prompting for a password. If 0 non password
 * prompt is used.
 * @param optional indicating if the field is optional or not.
 */
void promptAndSet(struct oidc_account* account, char* prompt_str,
                  void (*set_callback)(struct oidc_account*, char*),
                  char* (*get_callback)(const struct oidc_account*),
                  int passPrompt, int optional) {
  char* input = NULL;
  do {
    if (passPrompt) {
      input = promptPassword(prompt_str,
                             strValid(get_callback(account)) ? " [***]" : "");
    } else {
      input =
          prompt(prompt_str, strValid(get_callback(account)) ? " [" : "",
                 strValid(get_callback(account)) ? get_callback(account) : "",
                 strValid(get_callback(account)) ? "]" : "");
    }
    if (strValid(input)) {
      set_callback(account, input);
    } else {
      secFree(input);
    }
    if (optional) {
      break;
    }
  } while (!strValid(get_callback(account)));
}

void promptAndSetClientId(struct oidc_account* account) {
  promptAndSet(account, "Client_id%s%s%s: ", account_setClientId,
               account_getClientId, 0, 0);
}

void promptAndSetClientSecret(struct oidc_account* account, int usePubclient) {
  promptAndSet(account, "Client_secret%s: ", account_setClientSecret,
               account_getClientSecret, 1, usePubclient);
}

void promptAndSetScope(struct oidc_account* account) {
  // TODO --pub set, than should use the max of this public client
  char* supportedScope =
      compIssuerUrls(account_getIssuerUrl(account), ELIXIR_ISSUER_URL)
          ? oidc_strcopy(ELIXIR_SUPPORTED_SCOPES)
          : gen_handleScopeLookup(account_getIssuerUrl(account));
  printNormal("This issuer supports the following scopes: %s\n",
              supportedScope);
  if (!strValid(account_getScope(account))) {
    account_setScope(account, oidc_strcopy(DEFAULT_SCOPE));
  }
  promptAndSet(account, "Space delimited list of scopes or 'max'%s%s%s: ",
               account_setScopeExact, account_getScope, 0, 0);
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
    promptAndSet(account, "Refresh token%s%s%s: ", account_setRefreshToken,
                 account_getRefreshToken, 0, 0);
  }
}

void promptAndSetUsername(struct oidc_account* account, list_t* flows) {
  if (findInList(flows, FLOW_VALUE_PASSWORD)) {
    promptAndSet(account, "Username%s%s%s: ", account_setUsername,
                 account_getUsername, 0, 0);
  }
}

void promptAndSetPassword(struct oidc_account* account, list_t* flows) {
  if (findInList(flows, FLOW_VALUE_PASSWORD)) {
    promptAndSet(account, "Password%s: ", account_setPassword,
                 account_getPassword, 1, 0);
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
    promptAndSet(account, "Cert Path%s%s%s: ", account_setCertPath,
                 account_getCertPath, 0, 0);
  }
}

void promptAndSetName(struct oidc_account* account, const char* short_name,
                      const struct optional_arg cnid) {
  char* shortname = oidc_strcopy(short_name);
  while (!strValid(shortname)) {
    secFree(shortname);
    shortname = prompt("Enter short name for the account to configure: ");
  }
  char* client_identifier =
      cnid.useIt
          ? cnid.str
                ? oidc_strcopy(cnid.str)
                : prompt("Enter optional additional client-name-identifier: ")
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
  char* input   = NULL;
  char* arr_str = listToDelimitedString(account_getRedirectUris(account), ' ');
  oidc_error_t err;
  do {
    input = prompt("Space separated redirect_uris [%s]: ",
                   strValid(arr_str) ? arr_str : "");
    if (strValid(input)) {
      list_t* redirect_uris = delimitedStringToList(input, ' ');
      secFree(input);
      err = checkRedirectUrisForErrors(redirect_uris);
      if (err != OIDC_SUCCESS) {
        list_destroy(redirect_uris);
        continue;
      }
      account_setRedirectUris(account, redirect_uris);
    } else {
      secFree(input);
      err = checkRedirectUrisForErrors(account_getRedirectUris(account));
      if (err == OIDC_EERROR) {
        account_setRedirectUris(account, NULL);
        secFree(arr_str);
        arr_str = oidc_strcopy("");
        continue;
      }
    }
    if (account_refreshTokenIsValid(account) ||
        (strValid(account_getUsername(account)) &&
         strValid(account_getPassword(account))) ||
        useDevice) {
      break;  // redirect_uris only required if no refresh token and no user
              // credentials provided
    }
    secFree(arr_str);
    arr_str = listToDelimitedString(account_getRedirectUris(account), ' ');
  } while (!strValid(arr_str) || err != OIDC_SUCCESS);
  secFree(arr_str);
}

int _promptIssuer(struct oidc_account* account, const char* fav) {
  char* input = prompt("Issuer [%s]: ", fav);
  if (!strValid(input)) {
    char* iss = oidc_strcopy(fav);
    secFree(input);
    struct oidc_issuer* issuer = secAlloc(sizeof(struct oidc_issuer));
    issuer_setIssuerUrl(issuer, iss);
    account_setIssuer(account, issuer);
    return -1;
  } else if (isdigit(*input)) {
    int i = strToInt(input);
    secFree(input);
    i--;  // printed indices starts at 1 for non nerds
    return i;
  } else {
    struct oidc_issuer* issuer = secAlloc(sizeof(struct oidc_issuer));
    issuer_setIssuerUrl(issuer, input);
    account_setIssuer(account, issuer);
    return -1;
  }
}

void _useSuggestedIssuer(struct oidc_account* account) {
  list_t* issuers = getSuggestableIssuers();
  char*   fav     = getFavIssuer(account, issuers);
  printSuggestIssuer(issuers);
  int i;
  while ((i = _promptIssuer(account, fav)) >= (int)issuers->len) {
    printError("input out of bound\n");
  }
  if (i >= 0) {
    struct oidc_issuer* issuer = secAlloc(sizeof(struct oidc_issuer));
    issuer_setIssuerUrl(issuer, oidc_strcopy(list_at(issuers, i)->val));
    account_setIssuer(account, issuer);
  }
  list_destroy(issuers);
}

void promptAndSetIssuer(struct oidc_account* account) {
  if (!oidcFileDoesExist(ISSUER_CONFIG_FILENAME) &&
      !fileDoesExist(ETC_ISSUER_CONFIG_FILE)) {
    promptAndSet(account, "Issuer%s%s%s: ", account_setIssuerUrl,
                 account_getIssuerUrl, 0, 0);
  } else {
    _useSuggestedIssuer(account);
  }
  stringifyIssuerUrl(account);
}
