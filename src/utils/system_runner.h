#ifndef OIDC_SYSTEM_RUNNER_H
#define OIDC_SYSTEM_RUNNER_H

char* getOutputFromCommand(const char* cmd);
void  fireCommand(const char* cmd);
int   openUrlInBrowser(const char* url);

#endif  // OIDC_SYSTEM_RUNNER_H
