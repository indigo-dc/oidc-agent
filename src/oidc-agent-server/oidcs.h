#ifndef OIDC_SERVER_DAEMON_H
#define OIDC_SERVER_DAEMON_H

#include "defines/version.h"
#include "ipc/pipe.h"
#include "ipc/serveripc.h"

const char* argp_program_version = AGENTSERVER_VERSION;

const char* argp_program_bug_address = BUG_ADDRESS;

void handleAdd(int sock, const char* config, const char* data_dir);
void handleRemove(int sock, const char* id, const char* data_dir);
void handleToken(struct ipcPipe pipes, int sock, const char* id,
                 const char* complete_request, const char* data_dir);
void handleClientComm(struct connection* listencon, struct ipcPipe pipes,
                      const char* data_dir);

#endif /* OIDC_SERVER_DAEMON_H */
