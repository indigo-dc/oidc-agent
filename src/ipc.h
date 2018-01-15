#ifndef IPC_H
#define IPC_H

#include <sys/un.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>

#include <syslog.h>

#include "oidc_array.h"
#include "oidc_error.h"

#define RESPONSE_SUCCESS_CLIENT "{\n\"status\":\"success\",\n\"client\":%s\n}"
#define RESPONSE_ERROR_CLIENT_INFO "{\n\"status\":\"failure\",\n\"error\":\"%s\",\n\"client\":%s,\n\"info\":\"%s\"\n}"
#define RESPONSE_STATUS_SUCCESS "{\n\"status\":\"success\"\n}"
#define RESPONSE_STATUS_CONFIG "{\n\"status\":\"%s\",\n\"config\":%s\n}"
#define RESPONSE_STATUS_ACCESS "{\n\"status\":\"%s\",\n\"access_token\":\"%s\"\n}"
#define RESPONSE_STATUS_ACCOUNT "{\n\"status\":\"%s\",\n\"account_list\":%s\n}"
#define RESPONSE_STATUS_REGISTER "{\n\"status\":\"%s\",\n\"response\":%s\n}"
#define RESPONSE_STATUS_CODEURI "{\n\"status\":\"%s\",\n\"uri\":\"%s\"\n}"
#define RESPONSE_STATUS_CODEURI_INFO "{\n\"status\":\"%s\",\n\"uri\":\"%s\",\n\"info\":\"%s\"\n}"
#define RESPONSE_ERROR "{\n\"status\":\"failure\",\n\"error\":\"%s\"\n}"

#define REQUEST "{\n\"request\":\"%s\",\n%s\n}"
#define REQUEST_CONFIG "{\n\"request\":\"%s\",\n\"config\":%s\n}"
#define REQUEST_CONFIG_FLOW "{\n\"request\":\"%s\",\n\"config\":%s,\n\"flow\":\"%s\"\n}"
#define REQUEST_CODEEXCHANGE "{\n\"request\":\"code_exchange\",\n\"config\":%s,\n\"redirect_uri\":\"%s\",\n\"code\":\"%s\"\n}"

struct connection {
  int* sock;
  int* msgsock;
  struct sockaddr_un* server;
};

char* init_socket_path(const char* env_var_name) ;
oidc_error_t ipc_init(struct connection* con, const char* env_var_name, int isServer) ;
oidc_error_t ipc_initWithPath(struct connection* con) ;
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
