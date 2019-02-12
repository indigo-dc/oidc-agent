#ifndef PORT_UTILS_H
#define PORT_UTILS_H

#include "account/account.h"
#include "utils/oidc_error.h"

#define MAX_PORT 49151
#define MIN_PORT 1024

unsigned short getRandomPort();
char*          portToUri(unsigned short port);
unsigned int   getPortFromUri(const char* uri);
char* findRedirectUriByPort(const struct oidc_account* a, unsigned short port);
oidc_error_t portIsInRange(unsigned short port);

#endif  // PORT_UTILS_H
