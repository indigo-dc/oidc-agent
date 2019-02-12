#ifndef OIDC_GEN__SIGNAL_HANDLER_H
#define OIDC_GEN__SIGNAL_HANDLER_H

void registerSignalHandler(const char* state);
void gen_http_signal_handler(int signo);
void unregisterSignalHandler();

#endif  // OIDC_GEN__SIGNAL_HANDLER_H
