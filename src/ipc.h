#ifndef IPC_H
#define IPC_H

#include <sys/un.h>
#include <time.h>

// modes
#define PROMPT 3
#define PROMPT_PASSWORD 2
#define PRINT 1
#define PRINT_AND_CLOSE 0
#define PROMPT_CHAR PROMPT + '0'
#define PROMPT_PASSWORD_CHAR PROMPT_PASSWORD + '0'
#define PRINT_CHAR PRINT + '0'
#define PRINT_AND_CLOSE_CHAR PRINT_AND_CLOSE + '0'

#define DAEMON_NOT_RUNNING 1

struct connection {
  int* sock;
  int* msgsock;
  struct sockaddr_un* server;
  char* dir;
};

char* init_socket_path(struct connection* con, const char* prefix, const char* env_var_name) ;
int ipc_init(struct connection* con, const char* prefix, const char* env_var_name, int isServer) ;
int ipc_bind(struct connection* con, void(callback)()) ;
int ipc_bindAndListen(struct connection* con) ;
int ipc_accept_async(struct connection* con, time_t timeout_s) ;
int ipc_connect(struct connection con) ;
char* ipc_read(int _sock);
int ipc_write(int _sock, char* msg, ...);
int ipc_writeWithMode(int _sock, int mode, char* msg, ...) ;
int ipc_close(struct connection* con);
int ipc_closeAndUnlink(struct connection* con);

#endif // IPC_H
