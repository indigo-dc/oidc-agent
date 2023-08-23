#include "askpass.h"

#include "oidc-agent/oidcp/passwords/agent_prompt.h"
#include "utils/agentLogger.h"
#include "utils/json.h"
#include "utils/memory.h"
#include "utils/oidc_error.h"
#include "utils/prompting/getprompt.h"
#include "utils/string/stringUtils.h"

char* askpass_getPasswordForUpdate(const char* shortname) {
  if (shortname == NULL) {
    oidc_setArgNullFuncError(__func__);
    return NULL;
  }
  agent_log(
      DEBUG,
      "Prompting user for encryption password for updating account config '%s'",
      shortname);
  cJSON* data = generateJSONObject("shortname", cJSON_String, shortname, NULL);
  char*  msg  = getprompt(PROMPTTEMPLATE(UPDATE_ACCOUNT), data);
  secFreeJson(data);
  char* ret = agent_promptPassword(msg, "Encryption password", NULL);
  secFree(msg);
  if (ret == NULL) {
    oidc_errno = OIDC_EUSRPWCNCL;
  }
  return ret;
}

char* askpass_getPasswordForAutoload(const char* shortname,
                                     const char* application_hint) {
  if (shortname == NULL) {
    oidc_setArgNullFuncError(__func__);
    return NULL;
  }
  agent_log(DEBUG,
            "Prompting user for encryption password for autoload config '%s'",
            shortname);
  cJSON* data = generateJSONObject("shortname", cJSON_String, shortname, NULL);
  data        = jsonAddStringValue(data, "application-hint", application_hint);
  char* msg   = getprompt(PROMPTTEMPLATE(UNLOCK_ACCOUNT), data);
  secFreeJson(data);
  char* ret = agent_promptPassword(msg, "Encryption password", NULL);
  secFree(msg);
  if (ret == NULL) {
    oidc_errno = OIDC_EUSRPWCNCL;
  }
  return ret;
}

char* askpass_getPasswordForAutoloadWithIssuer(const char* issuer,
                                               const char* shortname,
                                               const char* application_hint) {
  if (shortname == NULL) {
    oidc_setArgNullFuncError(__func__);
    return NULL;
  }
  agent_log(
      DEBUG,
      "Prompting user for encryption password for autoload config '%s' for "
      "issuer '%s'",
      shortname, issuer);
  cJSON* data = generateJSONObject("shortname", cJSON_String, shortname,
                                   "issuer", cJSON_String, issuer, NULL);
  data        = jsonAddStringValue(data, "application-hint", application_hint);
  char* msg   = getprompt(PROMPTTEMPLATE(UNLOCK_ACCOUNT_ISSUER), data);
  secFreeJson(data);
  char* ret = agent_promptPassword(msg, "Encryption password", NULL);
  secFree(msg);
  if (ret == NULL) {
    oidc_errno = OIDC_EUSRPWCNCL;
  }
  return ret;
}

oidc_error_t _askpass_getConfirmation(const char* issuer, const char* shortname,
                                      const char* application_hint,
                                      const char* token_type) {
  cJSON* data =
      generateJSONObject("shortname", cJSON_String, shortname, "token-type",
                         cJSON_String, token_type, NULL);
  data = jsonAddStringValue(data, "issuer", issuer);
  data = jsonAddStringValue(data, "application-hint", application_hint);
  if (strcaseequal(token_type, "id")) {
    data = cJSON_AddBoolToObject(data, "id", cJSON_True);
  }
  char* msg = getprompt(PROMPTTEMPLATE(CONFIRM), data);
  secFreeJson(data);
  oidc_errno =
      agent_promptConsentDefaultYes(msg) ? OIDC_SUCCESS : OIDC_EFORBIDDEN;
  secFree(msg);
  return oidc_errno;
}

oidc_error_t askpass_getConfirmation(const char* shortname,
                                     const char* application_hint) {
  if (shortname == NULL) {
    oidc_setArgNullFuncError(__func__);
    return oidc_errno;
  }
  agent_log(DEBUG, "Prompting user for confirmation of using config '%s'",
            shortname);
  return _askpass_getConfirmation(NULL, shortname, application_hint, "access");
}

oidc_error_t askpass_getConfirmationWithIssuer(const char* issuer,
                                               const char* shortname,
                                               const char* application_hint) {
  if (shortname == NULL) {
    oidc_setArgNullFuncError(__func__);
    return oidc_errno;
  }
  agent_log(
      DEBUG,
      "Prompting user for confirmation of using config '%s' for issuer '%s'",
      shortname, issuer);
  return _askpass_getConfirmation(issuer, shortname, application_hint,
                                  "access");
}

oidc_error_t askpass_getIdTokenConfirmation(const char* shortname,
                                            const char* application_hint) {
  if (shortname == NULL) {
    oidc_setArgNullFuncError(__func__);
    return oidc_errno;
  }
  agent_log(DEBUG, "Prompting user for id-token confirmation for config '%s'",
            shortname);
  return _askpass_getConfirmation(NULL, shortname, application_hint, "id");
}

oidc_error_t askpass_getIdTokenConfirmationWithIssuer(
    const char* issuer, const char* shortname, const char* application_hint) {
  agent_log(DEBUG,
            "Prompting user for id-token confirmation for "
            "issuer '%s'",
            issuer);
  return _askpass_getConfirmation(issuer, shortname, application_hint, "id");
}

char* askpass_getMytokenConfirmation(const char* base64_html) {
  agent_log(DEBUG, "Prompting user for mytoken confirmation");
  char* res  = agent_promptMytokenConsent(base64_html);
  oidc_errno = res != NULL ? OIDC_SUCCESS : OIDC_EFORBIDDEN;
  return res;
}
