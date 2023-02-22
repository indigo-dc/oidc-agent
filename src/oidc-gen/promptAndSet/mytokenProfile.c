#include "_helper.h"
#include "account/account.h"
#include "promptAndSet.h"
#include "utils/json.h"
#include "utils/string/stringUtils.h"

int parseAndSetProfile(struct oidc_account* account, char* profile) {
  if (profile == NULL) {
    return 0;
  }
  cJSON* p = stringToJsonDontLogError(profile);
  if (!(cJSON_IsObject(p) || cJSON_IsArray(p))) {
    secFreeJson(p);
    char* quotedProfile = oidc_sprintf("\"%s\"", profile);
    p                   = stringToJson(quotedProfile);
    secFree(quotedProfile);
  }
  secFree(profile);
  if (p == NULL) {
    //    printError("Could not parse mytoken profile: %s\n", oidc_serror());
    return 0;
  }
  account_setUsedMytokenProfile(account, jsonToStringUnformatted(p));
  secFreeJson(p);
  return 1;
}

void askOrNeedMyProfile(struct oidc_account*    account,
                        const struct arguments* arguments, int optional) {
  if (readMyProfile(account, arguments)) {
    return;
  }
  ERROR_IF_NO_PROMPT(optional,
                     ERROR_MESSAGE("mytoken profile", OPT_LONG_MYTOKENPROFILE));
  char* res = _gen_prompt("Mytoken Profile",
                          account_getUsedMytokenProfile(account), 0, optional);
  parseAndSetProfile(account, res);
}

int readMyProfile(struct oidc_account*    account,
                  const struct arguments* arguments) {
  if (arguments->mytoken_profile) {
    return parseAndSetProfile(account,
                              oidc_strcopy(arguments->mytoken_profile));
  }
  if (prompt_mode() == 0 && strValid(account_getUsedMytokenProfile(account))) {
    return 1;
  }
  return 0;
}

void askMyProfile(struct oidc_account*    account,
                  const struct arguments* arguments) {
  return askOrNeedMyProfile(account, arguments, 1);
}

void needMyProfile(struct oidc_account*    account,
                   const struct arguments* arguments) {
  return askOrNeedMyProfile(account, arguments, 0);
}
