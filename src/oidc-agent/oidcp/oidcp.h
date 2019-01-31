#ifndef OIDC_PROXY_DAEMON_H
#define OIDC_PROXY_DAEMON_H

#include "defines/version.h"
#include "ipc/pipe.h"
#include "ipc/serveripc.h"
#include "oidc-agent/oidc-agent_options.h"

const char* argp_program_version = AGENT_VERSION;

const char* argp_program_bug_address = BUG_ADDRESS;

void handleOidcdComm(struct ipcPipe pipes, int sock, const char* msg);
void handleClientComm(struct connection* listencon, struct ipcPipe pipes);

#endif  // OIDC_PROXY_DAEMON_H
