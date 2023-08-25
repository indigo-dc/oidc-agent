#include "_helper.h"
#include "account/account.h"
#include "promptAndSet.h"
#include "utils/file_io/oidc_file_io.h"
#include "utils/json.h"
#include "utils/prompting/getprompt.h"
#include "utils/prompting/prompt.h"
#include "utils/string/stringUtils.h"

void askOrNeedName(struct oidc_account* account, const char* arg0,
                   const char* cnid, unsigned char optional,
                   unsigned char shouldNotExist, const char* suggestion) {
  if (readName(account, arg0, cnid)) {
    if (!shouldNotExist || !oidcFileDoesExist(account_getName(account))) {
      return;
    }
  }
  ERROR_IF_NO_PROMPT(optional, "No account short name given.");
  char*         shortname = NULL;
  unsigned char exists    = strValid(account_getName(account))
                                ? oidcFileDoesExist(account_getName(account))
                                : 0;
  cJSON*        data      = cJSON_CreateObject();
  if (exists) {
    data = jsonAddBoolValue(data, "exists", cJSON_True);
  }
  do {
    secFree(shortname);
    char* text = getprompt(PROMPTTEMPLATE(SHORTNAME), data);
    shortname  = prompt(text, "short name", suggestion, CLI_PROMPT_VERBOSE);
    secFree(text);
    secFreeJson(data);
    exists = strValid(shortname) ? oidcFileDoesExist(shortname) : 0;
    data   = cJSON_CreateObject();
    if (exists) {
      data = jsonAddBoolValue(data, "exists", cJSON_True);
    }
  } while ((!strValid(shortname) && !optional) || (exists && shouldNotExist));
  secFreeJson(data);
  if (shortname) {
    account_setName(account, shortname, cnid);
  }
}

int readName(struct oidc_account* account, const char* arg0, const char* cnid) {
  if (arg0) {
    account_setName(account, oidc_strcopy(arg0), cnid);
    return 1;
  }
  if (prompt_mode() == 0 && strValid(account_getName(account))) {
    return 1;
  }
  return 0;
}

void askName(struct oidc_account* account, unsigned char shouldNotExist,
             const char* arg0, const char* cnid) {
  return askOrNeedName(account, arg0, cnid, 1, shouldNotExist, NULL);
}

void needName(struct oidc_account* account, unsigned char shouldNotExist,
              const char* arg0, const char* cnid) {
  return askOrNeedName(account, arg0, cnid, 0, shouldNotExist, NULL);
}
