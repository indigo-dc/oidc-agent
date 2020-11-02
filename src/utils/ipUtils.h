#ifndef OIDC_IP_UTILS_H
#define OIDC_IP_UTILS_H

int   isValidIP(const char* ipAddress);
char* hostnameToIP(const char* hostname);
int   isValidIPOrHostname(const char* iph);
int   isValidIPOrHostnameOptionalPort(const char* iph);

#endif /* OIDC_IP_UTILS_H */
