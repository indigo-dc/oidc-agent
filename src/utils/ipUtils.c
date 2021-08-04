#define _POSIX_C_SOURCE 200112L
#include "ipUtils.h"

#include <arpa/inet.h>
#include <netdb.h>
#include <stddef.h>
#include <string.h>

#include "utils/memory.h"
#include "utils/stringUtils.h"

int isValidIP(const char* ipAddress) {
  if (ipAddress == NULL) {
    return 0;
  }
  struct sockaddr_in sa;
  return inet_pton(AF_INET, ipAddress, &(sa.sin_addr)) != 0;
}

char* hostnameToIP(const char* hostname) {
  if (hostname == NULL) {
    return NULL;
  }
  struct addrinfo* servinfo;
  struct addrinfo  hints = {.ai_family = AF_INET, .ai_socktype = SOCK_STREAM};

  if (getaddrinfo(hostname, NULL, &hints, &servinfo) != 0) {
    return NULL;
  }

  char* ip = NULL;
  // loop through all the results and connect to the first we can
  for (struct addrinfo* p = servinfo; p != NULL; p = p->ai_next) {
    struct sockaddr_in* h = (struct sockaddr_in*)p->ai_addr;
    ip                    = inet_ntoa(h->sin_addr);
  }

  freeaddrinfo(servinfo);  // all done with this structure
  return ip;
}

// Checks if the given string is a valid ip or a hsotname that resolves to an ip
int isValidIPOrHostname(const char* iph) {
  if (iph == NULL) {
    return 0;
  }
  if (isValidIP(iph)) {
    return 1;
  }
  return hostnameToIP(iph) != NULL;
}

// Checks if the given string is a valid ip or a hsotname that resolves to an ip
// The given string might have an port, e.g.
// <ip/host> or <ip/host>:<port>
int isValidIPOrHostnameOptionalPort(const char* iph) {
  char* tmp = oidc_strcopy(iph);
  char* ip  = strtok(tmp, ":");
  int   ret = isValidIPOrHostname(ip);
  secFree(tmp);
  return ret;
}
