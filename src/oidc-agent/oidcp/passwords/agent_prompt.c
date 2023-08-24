#define _GNU_SOURCE
#include "agent_prompt.h"

#include <signal.h>

#include "oidc-agent/oidc/device_code.h"
#include "oidc-gen/qr.h"
#include "utils/json.h"
#include "utils/prompting/getprompt.h"
#include "utils/prompting/prompt.h"
#include "utils/string/stringUtils.h"

char* agent_promptPassword(const char* text, const char* label,
                           const char* init) {
// _promptPasswordGUI might raise SIGINT (if user cancels), oidcp should not
// crash then
#ifndef __APPLE__
  static sighandler_t old_sigint;
#else
  static sig_t old_sigint;
#endif
  old_sigint = signal(SIGINT, SIG_IGN);
  char* ret  = _promptPasswordGUI(text, label, init, AGENT_PROMPT_TIMEOUT);
  signal(SIGINT, old_sigint);
  return ret;
}

int agent_promptConsentDefaultYes(const char* text) {
  return _promptConsentGUIDefaultYes(text, AGENT_PROMPT_TIMEOUT);
}

char* agent_promptMytokenConsent(const char* base64_html) {
  return promptMytokenConsentGUI(base64_html, AGENT_PROMPT_TIMEOUT);
}

static const char* const intro_fmt =
    "An error occurred while using the '%s' account configuration.\n"
    "Most likely the refresh token expired. To solve the problem you have to "
    "re-authenticate.\n";

void agent_displayDeviceCode(const struct oidc_device_code* device,
                             const char*                    shortname,
                             unsigned char                  reauth_intro) {
  const char* qr  = "/tmp/oidc-qr";
  const char* url = strValid(oidc_device_getVerificationUriComplete(*device))
                        ? oidc_device_getVerificationUriComplete(*device)
                        : oidc_device_getVerificationUri(*device);
  if (getIMGQRCode(url, qr)) {
    qr = NULL;
  }

  char* intro =
      reauth_intro ? oidc_sprintf(intro_fmt, shortname) : oidc_strcopy("");
  cJSON* data = generateJSONObject("intro", cJSON_String, intro, NULL);
  data = jsonAddStringValue(data, "code", oidc_device_getUserCode(*device));
  if (qr != NULL) {
    data = jsonAddBoolValue(data, "qr", cJSON_True);
  }
  char* text = getprompt(PROMPTTEMPLATE(AUTHENTICATE), data);
  secFree(intro);
  secFreeJson(data);
  displayLinkGUI(text, url, qr);
  secFree(text);
}

void agent_displayAuthCodeURL(const char* url, const char* shortname,
                              unsigned char reauth_intro) {
  char* intro =
      reauth_intro ? oidc_sprintf(intro_fmt, shortname) : oidc_strcopy("");
  cJSON* data = generateJSONObject("intro", cJSON_String, intro, NULL);
  char*  text = getprompt(PROMPTTEMPLATE(AUTHENTICATE), data);
  secFree(intro);
  secFreeJson(data);
  displayLinkGUI(text, url, NULL);
  secFree(text);
}
