#include "askpass.h"
#include "utils/memory.h"
#include "utils/oidc_error.h"
#include "utils/stringUtils.h"
#include "utils/system_runner.h"

#include <syslog.h>

char* _promptForPassword(const char* prompt_msg) {
  char* cmd = oidc_sprintf("ssh-askpass \"%s\"", prompt_msg);
  char* ret = getOutputFromCommand(cmd);
  secFree(cmd);
  return ret;
}

char* askpass_getPasswordForUpdate(const char* shortname) {
  if (shortname == NULL) {
    oidc_setArgNullFuncError(__func__);
    return NULL;
  }
  syslog(
      LOG_AUTHPRIV | LOG_DEBUG,
      "Prompting user for encryption password for updating account config '%s'",
      shortname);
  const char* const fmt =
      "oidc-agent needs to update the account config for '%s'.\nPlease enter "
      "the encryption password for '%s':";
  char* msg = oidc_sprintf(fmt, shortname, shortname);
  char* ret = _promptForPassword(msg);
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
  syslog(LOG_AUTHPRIV | LOG_DEBUG,
         "Prompting user for encryption password for autoload config '%s'",
         shortname);
  const char* const fmt =
      "An application %srequests an access token for '%s'. This configuration "
      "is currently not loaded.\nTo load '%s' into oidc-agent please enter "
      "the encryption password for '%s':";
  char* application_str =
      application_hint ? oidc_sprintf("(%s) ", application_hint) : NULL;
  char* msg =
      oidc_sprintf(fmt, application_str ?: "", shortname, shortname, shortname);
  secFree(application_str);
  char* ret = _promptForPassword(msg);
  secFree(msg);
  if (ret == NULL) {
    oidc_errno = OIDC_EUSRPWCNCL;
  }
  return ret;
}

int askpass_promptConfirmation(const char* prompt_msg) {
  char* cmd = oidc_sprintf("ssh-askpass \"%s\"", prompt_msg);
  char* ret = getOutputFromCommand(cmd);
  secFree(cmd);
  return ret != NULL;
}
