#ifndef IPC_H
#define IPC_H

#include <sys/un.h>
#include <time.h>


// #define STATUS_SUCCESS "0"
// #define STATUS_SUCCESS_WITH_REFRESH "1"
// #define STATUS_FAILURE "3"
// #define ENV_VAR_NOT_SET 4
// #define JSON_MALFORMED "5"

#define RESPONSE_STATUS "{\"status\":\"%s\"}"
#define RESPONSE_STATUS_REFRESH "{\"status\":\"%s\", \"refresh_token\":\"%s\"}"
#define RESPONSE_STATUS_ACCESS "{\"status\":\"%s\", \"access_token\":\"%s\"}"
#define RESPONSE_ERROR "{\"status\":\"failure\", \"error\":\"%s\"}"


struct connection {
  int* sock;
  int* msgsock;
  struct sockaddr_un* server;
  char* dir;
};

char* init_socket_path(struct connection* con, const char* prefix, const char* env_var_name) ;
int ipc_init(struct connection* con, const char* prefix, const char* env_var_name, int isServer) ;
int ipc_bind(struct connection* con) ;
int ipc_bindAndListen(struct connection* con) ;
int ipc_accept_async(struct connection* con, time_t timeout_s) ;
int ipc_connect(struct connection con) ;
char* ipc_read(int _sock);
int ipc_write(int _sock, char* msg, ...);
int ipc_close(struct connection* con);
int ipc_closeAndUnlink(struct connection* con, const char* env_var_name);

#endif // IPC_H
