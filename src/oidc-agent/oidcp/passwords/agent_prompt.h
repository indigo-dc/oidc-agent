#ifndef OIDCP_AGENT_PROMPT_H
#define OIDCP_AGENT_PROMPT_H

#include "oidc-agent/oidc/device_code.h"
#include "utils/prompting/prompt.h"

#define AGENT_PROMPT_TIMEOUT PROMPT_DEFAULT_TIMEOUT

char* agent_promptPassword(const char* text, const char* label,
                           const char* init);
int   agent_promptConsentDefaultYes(const char* text);
char* agent_promptMytokenConsent(const char* base64_html);

void agent_displayAuthCodeURL(const char* url, const char* shortname,
                              unsigned char reauth_intro);
void agent_displayDeviceCode(const struct oidc_device_code* device,
                             const char* shortname, unsigned char reauth_intro);

#endif /* OIDCP_AGENT_PROMPT_H */
