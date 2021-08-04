#ifndef OIDC_IPC_PIPE_H
#define OIDC_IPC_PIPE_H

#include <stdarg.h>
#include <time.h>

#include "utils/oidc_error.h"

struct ipcPipe {
  int rx;
  int tx;
};

struct pipeSet {
  struct ipcPipe pipe1;
  struct ipcPipe pipe2;
};

void           ipc_closePipes(struct ipcPipe);
struct pipeSet ipc_pipe_init();
struct ipcPipe toServerPipes(struct pipeSet);
struct ipcPipe toClientPipes(struct pipeSet);

oidc_error_t ipc_writeToPipe(struct ipcPipe, const char*, ...);
oidc_error_t ipc_vwriteToPipe(struct ipcPipe, const char*, va_list);
oidc_error_t ipc_writeOidcErrnoToPipe(struct ipcPipe);
char*        ipc_readFromPipe(struct ipcPipe);
char*        ipc_readFromPipeWithTimeout(struct ipcPipe, time_t);
char*        ipc_communicateThroughPipe(struct ipcPipe, const char*, ...);
char*        ipc_vcommunicateThroughPipe(struct ipcPipe, const char*, va_list);

#endif  // OIDC_IPC_PIPE_H
