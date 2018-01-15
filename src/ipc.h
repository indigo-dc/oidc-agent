#ifndef IPC_H
#define IPC_H

#include <sys/un.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>

#include <syslog.h>

#include "oidc_array.h"
#include "oidc_error.h"

#define RESPONSE_SUCCESS_CLIENT "{\"status\":\"success\", \"client\":%s}"
#define RESPONSE_ERROR_CLIENT_INFO "{\"status\":\"failure\", \"error\":\"%s\", \"client\":%s, \"info\":\"%s\"}"
#define RESPONSE_STATUS_SUCCESS "{\"status\":\"success\"}"
#define RESPONSE_STATUS_CONFIG "{\"status\":\"%s\", \"config\":%s}"
#define RESPONSE_STATUS_ACCESS "{\"status\":\"%s\", \"access_token\":\"%s\"}"
#define RESPONSE_STATUS_ACCOUNT "{\"status\":\"%s\", \"account_list\":%s}"
#define RESPONSE_STATUS_REGISTER "{\"status\":\"%s\", \"response\":%s}"
#define RESPONSE_STATUS_CODEURI "{\"status\":\"%s\", \"uri\":\"%s\"}"
#define RESPONSE_STATUS_CODEURI_INFO "{\"status\":\"%s\", \"uri\":\"%s\", \"info\":\"%s\"}"
#define RESPONSE_ERROR "{\"status\":\"failure\", \"error\":\"%s\"}"

#define REQUEST "{\"request\":\"%s\", %s}"
#define REQUEST_CONFIG "{\"request\":\"%s\", \"config\":%s}"
#define REQUEST_CONFIG_FLOW "{\"request\":\"%s\", \"config\":%s, \"flow\":\"%s\"}"
#define REQUEST_CODEEXCHANGE "{\"request\":\"code_exchange\", \"config\":%s, \"redirect_uri\":\"%s\", \"code\":\"%s\"}"

struct connection {
  int* sock;
  int* msgsock;
  struct sockaddr_un* server;
};

char* init_socket_path(const char* env_var_name) ;
oidc_error_t ipc_init(struct connection* con, const char* env_var_name, int isServer) ;
int ipc_bindAndListen(struct connection* con) ;
struct connection* ipc_async(struct connection listencon, struct connection** clientcons_addr, size_t* size) ;
int ipc_connect(struct connection con) ;
char* ipc_read(int _sock);
oidc_error_t ipc_write(int _sock, char* msg, ...);
oidc_error_t ipc_vwrite(int _sock, char* msg, va_list args);
oidc_error_t ipc_writeOidcErrno(int sock) ;
oidc_error_t ipc_close(struct connection* con);
oidc_error_t ipc_closeAndUnlink(struct connection* con);

struct connection* addConnection(struct connection* cons, size_t* size, struct connection client) ;
struct connection* findConnection(struct connection* cons, size_t size, struct connection key) ;
struct connection* removeConnection(struct connection* cons, size_t* size, struct connection* key) ;

char* server_socket_path;

#endif // IPC_H
