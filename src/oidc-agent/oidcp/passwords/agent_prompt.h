#ifndef OIDCP_AGENT_PROMPT_H
#define OIDCP_AGENT_PROMPT_H

char* agent_promptPassword(const char* text, const char* label,
                           const char* init);
int   agent_promptConsentDefaultYes(const char* text);

#endif /* OIDCP_AGENT_PROMPT_H */
