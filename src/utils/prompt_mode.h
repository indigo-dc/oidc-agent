#ifndef OIDC_PROMPT_MODE_H
#define OIDC_PROMPT_MODE_H

#define PROMPT_MODE_CLI 1
#define PROMPT_MODE_GUI 2

/** set_prompt_mode sets the prompt mode and password prompt mode, if a
 * password prompt mode that is different from the normal prompt mode should
 * be set, the @c pw_prompt_mode function has be called after this function
 */
void set_prompt_mode(unsigned char mode);
/** set_pw_prompt_mode sets only the pw_prompt_mode
 */
void          set_pw_prompt_mode(unsigned char mode);
unsigned char prompt_mode();
unsigned char pw_prompt_mode();

#endif  // OIDC_PROMPT_MODE_H
