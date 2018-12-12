#ifndef PORT_UTILS_H
#define PORT_UTILS_H

#include "../account/account.h"

#define MAX_PORT 49151
#define MIN_PORT 1024

unsigned short getRandomPort();
char*          portToUri(unsigned short port);
unsigned int   getPortFromUri(const char* uri);
char* findRedirectUriByPort(struct oidc_account a, unsigned short port);

#endif  // PORT_UTILS_H
