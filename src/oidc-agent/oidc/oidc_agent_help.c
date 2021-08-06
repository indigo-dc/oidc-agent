#include "oidc_agent_help.h"

#include "account/account.h"
#include "utils/oidc_error.h"
#include "utils/string/stringUtils.h"

const char* getHelp() {
  if (oidc_errno != OIDC_EOIDC && oidc_errno != OIDC_ENOREFRSH) {
    return NULL;
  }
  // We escape \n \t as \\n \\t etc. because these strings are sent in the json
  // ipc to clients
  const char* err = oidc_serror();
  if (oidc_errno == OIDC_ENOREFRSH || strstarts(err, "invalid_grant:")) {
    return "Most likely the refresh token expired. To create a new one, just "
           "run:\\n"
           "\\t$ oidc-gen <shortname> --reauthenticate\\n";
  }
  if (strstarts(err, "invalid_scope:")) {
    return "We cannot get these scopes with the current configuration. To get "
           "these scopes you might need to adapt the client configuration "
           "with\\n"
           "\\t$ oidc-gen -m <shortname>\\n"
           "but it also might be necessary to change the client configuration "
           "with the OpenID provider.\\n";
  }
  if (strstarts(err, "invalid_request")) {
    return "This is most likely a bug. Please hand in a bug report: "
           "https://github.com/indigo-dc/oidc-agent\\n";
  }
  if (strstarts(err, "invalid_client")) {
    return "Probably the OIDC client has been deleted. You must register a new "
           "client with the OpenID Provider manually and then use \\n"
           "\\t$ oidc-gen -m <shortname>\\n"
           "to update this account configuration, or you delete this account "
           "configuration and create a new one using dynamic client "
           "configuration (if supported):\\n"
           "\\t $ oidc-gen -d <shortname>\\n"
           "\\t $ oidc-gen <shortname> --iss=<issuer>\\n";
  }
  return NULL;
}

char* getHelpWithAccountInfo(struct oidc_account* account) {
  const char* helpFmt = getHelp();
  char* help = strreplace(helpFmt, "<shortname>", account_getName(account));
  char* tmp  = strreplace(help, "<issuer>", account_getIssuerUrl(account));
  secFree(help);
  help = tmp;
  return help;
}
