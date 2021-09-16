#ifndef OIDC_AGENT_OIDC_AGENT_HELP_H
#define OIDC_AGENT_OIDC_AGENT_HELP_H

#include "account/account.h"

char*       getHelpWithAccountInfo(struct oidc_account* account);
const char* getHelp();

#endif  // OIDC_AGENT_OIDC_AGENT_HELP_H
