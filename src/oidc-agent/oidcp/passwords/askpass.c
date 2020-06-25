#include "askpass.h"
#include "utils/agentLogger.h"
#include "utils/memory.h"
#include "utils/oidc_error.h"
#include "utils/prompt.h"
#include "utils/stringUtils.h"

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
      "oidc-agent needs to update the account config for '%s'.\nPlease enter "
      "the encryption password for '%s':";
  char* msg = oidc_sprintf(fmt, shortname, shortname);
  char* ret = _promptPasswordGUI(msg, "Encryption password", NULL);
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
      "An application %srequests an access token for '%s'. This configuration "
      "is currently not loaded.\nTo load '%s' into oidc-agent please enter "
      "the encryption password for '%s':";
  char* application_str = strValid(application_hint)
                              ? oidc_sprintf("(%s) ", application_hint)
                              : NULL;
  char* msg =
      oidc_sprintf(fmt, application_str ?: "", shortname, shortname, shortname);
  secFree(application_str);
  char* ret = _promptPasswordGUI(msg, "Encryption password", NULL);
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
      "An application %srequests an access token for '%s', but there's "
      "currently no account configuration loaded for this provider. The "
      "default account configuration for this provider is '%s'.\nTo load '%s' "
      "into oidc-agent please enter the encryption password for '%s':";
  char* application_str = strValid(application_hint)
                              ? oidc_sprintf("(%s) ", application_hint)
                              : NULL;
  char* msg = oidc_sprintf(fmt, application_str ?: "", issuer, shortname,
                           shortname, shortname);
  secFree(application_str);
  char* ret = _promptPasswordGUI(msg, "Encryption password", NULL);
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
  const char* const fmt = "An application %srequests an access token for '%s'. "
                          "Do you want to allow this usage?";
  char* application_str = strValid(application_hint)
                              ? oidc_sprintf("(%s) ", application_hint)
                              : NULL;
  char* msg = oidc_sprintf(fmt, application_str ?: "", shortname);
  secFree(application_str);
  oidc_error_t ret =
      _promptConsentGUIDefaultYes(msg) ? OIDC_SUCCESS : OIDC_EFORBIDDEN;
  secFree(msg);
  return ret;
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
  const char* const fmt = "An application %srequests an access token for '%s'. "
                          "Do you want to allow the usage of '%s'?";
  char* application_str = strValid(application_hint)
                              ? oidc_sprintf("(%s) ", application_hint)
                              : NULL;
  char* msg = oidc_sprintf(fmt, application_str ?: "", issuer, shortname);
  secFree(application_str);
  oidc_error_t ret =
      _promptConsentGUIDefaultYes(msg) ? OIDC_SUCCESS : OIDC_EFORBIDDEN;
  secFree(msg);
  return ret;
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
      "An application %srequests an id token for '%s'. "
      "id tokens should not be passed to other applications as authorization. "
      "Do you want to allow this usage?";
  char* application_str = strValid(application_hint)
                              ? oidc_sprintf("(%s) ", application_hint)
                              : NULL;
  char* msg = oidc_sprintf(fmt, application_str ?: "", shortname);
  secFree(application_str);
  oidc_error_t ret =
      _promptConsentGUIDefaultYes(msg) ? OIDC_SUCCESS : OIDC_EFORBIDDEN;
  secFree(msg);
  return ret;
}

oidc_error_t askpass_getIdTokenConfirmationWithIssuer(
    const char* issuer, const char* shortname, const char* application_hint) {
  agent_log(DEBUG,
            "Prompting user for id-token confirmation for "
            "issuer '%s'",
            issuer);
  const char* const fmt =
      "An application %srequests an id token for '%s'. "
      "id tokens should not be passed to other applications as authorization. "
      "Do you want to allow the usage of '%s'?";
  char* application_str = strValid(application_hint)
                              ? oidc_sprintf("(%s) ", application_hint)
                              : NULL;
  char* msg =
      oidc_sprintf(fmt, application_str ?: "", issuer, shortname ?: issuer);
  secFree(application_str);
  oidc_error_t ret =
      _promptConsentGUIDefaultYes(msg) ? OIDC_SUCCESS : OIDC_EFORBIDDEN;
  secFree(msg);
  return ret;
}
