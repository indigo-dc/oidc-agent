#define _GNU_SOURCE
#include "pipe.h"
#include "defines/ipc_values.h"
#include "ipc/ipc.h"
#include "utils/oidc_error.h"

#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

void ipc_closePipes(struct ipcPipe p) {
  close(p.rx);
  close(p.tx);
}

struct pipeSet ipc_pipe_init() {
  int fd1[2];
  int fd2[2];
#ifdef __APPLE__
  if (pipe(fd1) != 0) {
#else
  if (pipe2(fd1, O_DIRECT) != 0) {
#endif
    oidc_setErrnoError();
    return (struct pipeSet){{-1, -1}, {-1, -1}};
  }
#ifdef __APPLE__
  if (pipe(fd2) != 0) {
#else
  if (pipe2(fd2, O_DIRECT) != 0) {
#endif
    oidc_setErrnoError();
    return (struct pipeSet){{-1, -1}, {-1, -1}};
  }
  struct ipcPipe pipe1 = {fd1[0], fd1[1]};
  struct ipcPipe pipe2 = {fd2[0], fd2[1]};
  return (struct pipeSet){pipe1, pipe2};
}

struct ipcPipe toServerPipes(struct pipeSet pipes) {
  struct ipcPipe server;
  close(pipes.pipe1.tx);
  server.rx = pipes.pipe1.rx;
  close(pipes.pipe2.rx);
  server.tx = pipes.pipe2.tx;
  return server;
}

struct ipcPipe toClientPipes(struct pipeSet pipes) {
  struct ipcPipe client;
  close(pipes.pipe1.rx);
  client.tx = pipes.pipe1.tx;
  close(pipes.pipe2.tx);
  client.rx = pipes.pipe2.rx;
  return client;
}

oidc_error_t ipc_writeToPipe(struct ipcPipe pipes, const char* fmt, ...) {
  va_list args;
  va_start(args, fmt);
  oidc_error_t ret = ipc_vwriteToPipe(pipes, fmt, args);
  va_end(args);
  return ret;
}

oidc_error_t ipc_vwriteToPipe(struct ipcPipe pipes, const char* fmt,
                              va_list args) {
  return ipc_vwrite(pipes.tx, fmt, args);
}

oidc_error_t ipc_writeOidcErrnoToPipe(struct ipcPipe pipes) {
  return ipc_writeToPipe(pipes, RESPONSE_ERROR, oidc_serror());
}

char* ipc_readFromPipe(struct ipcPipe pipes) { return ipc_read(pipes.rx); }

char* ipc_readFromPipeWithTimeout(struct ipcPipe pipes, time_t timeout) {
  return ipc_readWithTimeout(pipes.rx, timeout);
}

char* ipc_vcommunicateThroughPipe(struct ipcPipe pipes, const char* fmt,
                                  va_list args) {
  if (ipc_vwriteToPipe(pipes, fmt, args) != OIDC_SUCCESS) {
    return NULL;
  }
  return ipc_readFromPipe(pipes);
}

char* ipc_communicateThroughPipe(struct ipcPipe pipes, const char* fmt, ...) {
  va_list args;
  va_start(args, fmt);
  char* ret = ipc_vcommunicateThroughPipe(pipes, fmt, args);
  va_end(args);
  return ret;
}
