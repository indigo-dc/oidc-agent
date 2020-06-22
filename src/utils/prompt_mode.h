#ifndef OIDC_PROMPT_MODE_H
#define OIDC_PROMPT_MODE_H

#define PROMPT_MODE_CLI 1
#define PROMPT_MODE_GUI 2

void          set_prompt_mode(unsigned char mode);
unsigned char prompt_mode();

#endif  // OIDC_PROMPT_MODE_H
