#ifndef PORT_UTILS_H
#define PORT_UTILS_H

#include "../account.h"

unsigned short getRandomPort();
char*          portToUri(unsigned short port);
unsigned short getPortFromUri(const char* uri);
char* findRedirectUriByPort(struct oidc_account a, unsigned short port);

#endif  // PORT_UTILS_H
