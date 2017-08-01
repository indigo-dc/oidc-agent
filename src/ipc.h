#ifndef IPC_H
#define IPC_H

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
};

char* init_socket_path(const char* prefix, const char* env_var_name) ;
int ipc_init(struct connection* con, const char* prefix, const char* env_var_name, int isServer) ;
int ipc_bind(struct connection con, void(callback)()) ;
int ipc_connect(struct connection con) ;
char* ipc_read(int _sock);
int ipc_write(int _sock, char* msg);
int ipc_writeWithMode(int _sock, char* msg, int mode) ;
int ipc_close(struct connection con);

#endif // IPC_H
