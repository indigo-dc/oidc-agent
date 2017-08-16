#ifndef OIDC_GEN_H
#define OIDC_GEN_H

#define DEFAULT_CA_PATH "/etc/ssl/certs/ca-certificates.crt"

struct oidc_provider* genNewProvider();
char* getTokenEndpoint(const char* configuration_endpoint);
char* encryptProvider(const char* json, const char* password) ;
void saveExit(int exitno);

#endif // OIDC_GEN_H
