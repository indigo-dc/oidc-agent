#ifndef OIDC_AGENT_H
#define OIDC_AGENT_H

#define OIDC_SOCK_ENV_NAME "OIDC_SOCK"
#define OIDC_PID_ENV_NAME "OIDCD_PID"

char* getTokenEndpoint(const char* configuration_endpoint, const char* cert_file);

#endif //OIDC_AGENT_H
