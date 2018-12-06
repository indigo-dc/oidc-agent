#ifndef OIDC_IPC_PIPE_H
#define OIDC_IPC_PIPE_H

struct pipe {
  int rx;
  int tx;
};

struct pipeSet {
  struct pipe pipe1;
  struct pipe pipe2;
};

struct ipcPipe {
  int rx;
  int tx;
};

void           closeIpcPipes(struct ipcPipe p);
struct pipeSet ipc_pipe_init();
struct ipcPipe toServerPipes(struct pipeSet pipes);
struct ipcPipe toClientPipes(struct pipeSet pipes);

#endif  // OIDC_IPC_PIPE_H
