#ifndef PARSE_OIDP_H
#define PARSE_OIDP_H

#include "account/account.h"
#include "utils/oidc_error.h"

char*                    parseForError(char* res);
struct oidc_device_code* parseDeviceCode(char* res);
oidc_error_t parseOpenidConfiguration(char* res, struct oidc_account* account);

#endif  // PARSE_OIDP_H
