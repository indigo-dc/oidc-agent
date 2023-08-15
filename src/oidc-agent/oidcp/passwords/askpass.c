#include "askpass.h"

#include "oidc-agent/oidcp/passwords/agent_prompt.h"
#include "utils/agentLogger.h"
#include "utils/memory.h"
#include "utils/oidc_error.h"
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
  const char* const fmt =
      "<h2>Update Account Config</h2"
      "<p/>oidc-agent needs to update the account config for <b>%s</b>.\nPlease enter "
      "the encryption password for <b>%s</b>:";
  char* msg = oidc_sprintf(fmt, shortname, shortname);
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
  const char* const fmt =
      "<h2>Unlock Identity</h2>"
      "<p/><b>%s</b> requests an access token for <b>%s</b>.\nThis configuration "
      "is currently not loaded.\nTo load <b>%s</b> into oidc-agent please enter "
      "the encryption password for <b>%s</b>:";
  char* application_str = strValid(application_hint)
                              ? oidc_sprintf("(%s) ", application_hint)
                              : NULL;
  char* msg =
      oidc_sprintf(fmt, application_str ?: "", shortname, shortname, shortname);
  secFree(application_str);
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
  const char* const fmt =
      "<h2>Unlock Identity</h2>"
      "<p/><b>%s</b>requests an access token for <b>%s</b>.\n"
      "Enter passphrase to unlock the identity <b>%s<b/> ";
  char* application_str = strValid(application_hint)
                              ? oidc_sprintf("(%s) ", application_hint)
                              : NULL;
  char* msg = oidc_sprintf(fmt, application_str ?: "", issuer, shortname);
  secFree(application_str);
  char* ret = agent_promptPassword(msg, "Encryption password", NULL);
  secFree(msg);
  if (ret == NULL) {
    oidc_errno = OIDC_EUSRPWCNCL;
  }
  return ret;
}

oidc_error_t askpass_getConfirmation(const char* shortname,
                                     const char* application_hint) {
  if (shortname == NULL) {
    oidc_setArgNullFuncError(__func__);
    return oidc_errno;
  }
  agent_log(DEBUG, "Prompting user for confirmation of using config '%s'",
            shortname);
  const char* const fmt =
      "<h2>Enter Password</h2>"
      "<p/><b>%s</b>requests an access token for <b>%s</b>.\n"
      "Do you want to allow this usage?";
  char* application_str = strValid(application_hint)
                              ? oidc_sprintf("(%s) ", application_hint)
                              : NULL;
  char* msg             = oidc_sprintf(fmt, application_str ?: "", shortname);
  secFree(application_str);
  oidc_errno =
      agent_promptConsentDefaultYes(msg) ? OIDC_SUCCESS : OIDC_EFORBIDDEN;
  secFree(msg);
  return oidc_errno;
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
  const char* const fmt =
      "<h2>Confirm</h2>"
      "<p/><b>%s</b>requests an access token for <b>%s</b>.\n"
      "Do you want to allow the usage of <b>%s</b>?";
  char* application_str = strValid(application_hint)
                              ? oidc_sprintf("(%s) ", application_hint)
                              : NULL;
  char* msg = oidc_sprintf(fmt, application_str ?: "", issuer, shortname);
  secFree(application_str);
  oidc_errno =
      agent_promptConsentDefaultYes(msg) ? OIDC_SUCCESS : OIDC_EFORBIDDEN;
  secFree(msg);
  return oidc_errno;
}

oidc_error_t askpass_getIdTokenConfirmation(const char* shortname,
                                            const char* application_hint) {
  if (shortname == NULL) {
    oidc_setArgNullFuncError(__func__);
    return oidc_errno;
  }
  agent_log(DEBUG, "Prompting user for id-token confirmation for config '%s'",
            shortname);
  const char* const fmt =
      "<h2>Confirm</h2>"
      "<p/><b>%s</b>requests an id token for <b>%s</b>.\n"
      "id tokens should not be passed to other applications as authorization.\n"
      "Do you want to allow this usage?";
  char* application_str = strValid(application_hint)
                              ? oidc_sprintf("(%s) ", application_hint)
                              : NULL;
  char* msg             = oidc_sprintf(fmt, application_str ?: "", shortname);
  secFree(application_str);
  oidc_errno =
      agent_promptConsentDefaultYes(msg) ? OIDC_SUCCESS : OIDC_EFORBIDDEN;
  secFree(msg);
  return oidc_errno;
}

oidc_error_t askpass_getIdTokenConfirmationWithIssuer(
    const char* issuer, const char* shortname, const char* application_hint) {
  agent_log(DEBUG,
            "Prompting user for id-token confirmation for "
            "issuer '%s'",
            issuer);
  const char* const fmt =
      "<h2>Confirm</h2>"
      "<b>%s</b>requests an id token for <b>%s</b>.\n"
      "id tokens should not be passed to other applications as authorization.\n"
      "Do you want to allow the usage of <b>%s</b>?";
  char* application_str = strValid(application_hint)
                              ? oidc_sprintf("(%s) ", application_hint)
                              : NULL;
  char* msg =
      oidc_sprintf(fmt, application_str ?: "", issuer, shortname ?: issuer);
  secFree(application_str);
  oidc_errno =
      agent_promptConsentDefaultYes(msg) ? OIDC_SUCCESS : OIDC_EFORBIDDEN;
  secFree(msg);
  return oidc_errno;
}

char* askpass_getMytokenConfirmation(const char* base64_html) {
  agent_log(DEBUG, "Prompting user for mytoken confirmation");
  char* res  = agent_promptMytokenConsent(base64_html);
  oidc_errno = res != NULL ? OIDC_SUCCESS : OIDC_EFORBIDDEN;
  return res;
}
