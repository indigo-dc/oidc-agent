#ifndef OIDC_PASSWORD_HANDLER_H
#define OIDC_PASSWORD_HANDLER_H

#include "utils/oidc_error.h"

#include <time.h>

#define PW_TYPE_MEM 0x01
#define PW_TYPE_MNG 0x02
#define PW_TYPE_CMD 0x04
#define PW_TYPE_PRMT 0x08

oidc_error_t savePasswordFor(const char* shortname, const char* password,
                             time_t expires_at, unsigned char type);
char*        getPasswordFor(const char* shortname);
oidc_error_t removePasswordFor(const char* shortname);

#endif  // OIDC_PASSWORD_HANDLER_H
