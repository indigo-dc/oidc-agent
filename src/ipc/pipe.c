#define _GNU_SOURCE
#include "pipe.h"
#include "utils/oidc_error.h"

#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

void closeIpcPipes(struct ipcPipe p) {
  close(p.rx);
  close(p.tx);
}

struct pipeSet ipc_pipe_init() {
  int fd1[2];
  int fd2[2];
  if (pipe2(fd1, O_DIRECT) != 0) {
    oidc_setErrnoError();
    return (struct pipeSet){{-1, -1}, {-1, -1}};
  }
  if (pipe2(fd2, O_DIRECT) != 0) {
    oidc_setErrnoError();
    return (struct pipeSet){{-1, -1}, {-1, -1}};
  }
  struct pipe pipe1 = {fd1[0], fd1[1]};
  struct pipe pipe2 = {fd2[0], fd2[1]};
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
