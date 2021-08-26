#ifndef OIDC_AGENT_CONFIG_UPDATER_H
#define OIDC_AGENT_CONFIG_UPDATER_H

#include "utils/oidc_error.h"

oidc_error_t updateRefreshTokenUsingPassword(const char* shortname,
                                             const char* encrypted_content,
                                             const char* refresh_token,
                                             const char* password);
oidc_error_t updateRefreshTokenUsingGPG(const char* shortname,
                                        const char* encrypted_content,
                                        const char* refresh_token,
                                        const char* gpg_key);
oidc_error_t writeOIDCFile(const char* content, const char* shortname);

#endif  // OIDC_AGENT_CONFIG_UPDATER_H
