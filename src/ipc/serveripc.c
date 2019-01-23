#define _XOPEN_SOURCE 700
#include "serveripc.h"
#include "cryptIpc.h"
#include "ipc.h"
#include "utils/json.h"
#include "utils/memory.h"

#include <sys/fcntl.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <syslog.h>
#include <time.h>
#include <unistd.h>

#define SOCKET_DIR "/tmp/oidc-XXXXXX"

static char* oidc_ipc_dir       = NULL;
static char* server_socket_path = NULL;

/**
 * @brief generates the socket path and prints commands for setting env vars
 * @param env_var_name the name of the environment variable which will be set.
 * If NULL non will be set.
 * @return a pointer to the socket_path. Has to be freed after usage.
 */
char* init_socket_path(const char* env_var_name) {
  if (NULL == oidc_ipc_dir) {
    oidc_ipc_dir = oidc_strcopy(SOCKET_DIR);
    if (mkdtemp(oidc_ipc_dir) == NULL) {
      syslog(LOG_AUTHPRIV | LOG_ALERT, "%m");
      oidc_errno = OIDC_EMKTMP;
      return NULL;
    }
  }
  pid_t ppid        = getppid();
  char* prefix      = "oidc-agent";
  char* fmt         = "%s/%s.%d";
  char* socket_path = oidc_sprintf(fmt, oidc_ipc_dir, prefix, ppid);
  if (env_var_name) {
    // printf("You have to set env var '%s' to '%s'. Please use the following
    // statement:\n", env_var_name, socket_path);
    printf("%s=%s; export %s;\n", env_var_name, socket_path, env_var_name);
  }
  return socket_path;
}

oidc_error_t initServerConnection(struct connection* con) {
  return initConnectionWithoutPath(con, 1);
}

/**
 * @brief initializes a server unix domain socket
 * @param con, a pointer to the connection struct. The relevant fields will be
 * initialized.
 * @param env_var_name, the socket_path environment variable name
 */
oidc_error_t ipc_server_init(struct connection* con, const char* env_var_name) {
  syslog(LOG_AUTHPRIV | LOG_DEBUG, "initializing server ipc");
  if (initServerConnection(con) != OIDC_SUCCESS) {  // TODO
    return oidc_errno;
  }
  char* path = init_socket_path(env_var_name);
  if (path == NULL) {
    return oidc_errno;
  }
  strcpy(con->server->sun_path, path);
  secFree(path);
  server_socket_path = con->server->sun_path;
  return OIDC_SUCCESS;
}

/**
 * @brief initializes unix domain socket with the current server_socket_path
 * @param con, a pointer to the connection struct. The relevant fields will be
 * initialized.
 * @return 0 on success, otherwise a negative error code
 */
oidc_error_t ipc_initWithPath(struct connection* con) {
  if (server_socket_path == NULL) {
    oidc_setArgNullFuncError(__func__);
    return oidc_errno;
  }
  syslog(LOG_AUTHPRIV | LOG_DEBUG, "initializing ipc with path %s\n",
         server_socket_path);
  if (initConnectionWithoutPath(con, 0) != OIDC_SUCCESS) {
    return oidc_errno;
  }
  strcpy(con->server->sun_path, server_socket_path);
  return OIDC_SUCCESS;
}

/**
 * @brief binds the server socket and starts listening
 * @param con, a pointer to the connection struct
 * @return @c 0 on success or an errorcode on failure
 */
int ipc_bindAndListen(struct connection* con) {
  syslog(LOG_AUTHPRIV | LOG_DEBUG, "binding ipc\n");
  unlink(con->server->sun_path);
  if (bind(*(con->sock), (struct sockaddr*)con->server,
           sizeof(struct sockaddr_un))) {
    syslog(LOG_AUTHPRIV | LOG_ALERT, "binding stream socket: %m");
    close(*(con->sock));
    oidc_errno = OIDC_EBIND;
    return OIDC_EBIND;
  }
  int flags;
  if (-1 == (flags = fcntl(*(con->sock), F_GETFL, 0)))
    flags = 0;
  fcntl(*(con->sock), F_SETFL, flags | O_NONBLOCK);

  syslog(LOG_AUTHPRIV | LOG_DEBUG, "listen ipc\n");
  return listen(*(con->sock), 5);
}

/**
 * @brief handles asynchronous server read for multiple sockets
 *
 * listens for incoming connections on the listencon and for incoming messages
 * on multiple client sockets. If a new client connects it is added to the list
 * of current client connections.  If on any client socket is a message
 * available for reading, a pointer to this connection is returned.
 * @param listencon the connection struct for the socket accepting new client
 * connections. The list is updated if a new client connects.
 * @param connections a list of client connections
 * @return A pointer to a client connection. On this connection is either a
 * message avaible for reading or the client disconnected.
 */
struct connection* ipc_readAsyncFromMultipleConnections(
    struct connection listencon, list_t* connections) {
  while (1) {
    fd_set readSockSet;
    FD_ZERO(&readSockSet);
    FD_SET(*(listencon.sock), &readSockSet);
    // Determine maxSock
    int          maxSock = *(listencon.sock);
    unsigned int i;
    for (i = 0; i < connections->len; i++) {
      struct connection* con = list_at(connections, i)->val;
      FD_SET(*(con->msgsock), &readSockSet);
      if (*(con->msgsock) > maxSock) {
        maxSock = *(con->msgsock);
      }
    }
    syslog(LOG_AUTHPRIV | LOG_DEBUG, "Calling select with maxSock %d", maxSock);
    // Waiting for incoming connections and messages
    int ret = select(maxSock + 1, &readSockSet, NULL, NULL, NULL);
    if (ret > 0) {
      if (FD_ISSET(*(listencon.sock),
                   &readSockSet)) {  // if listensock read something it means a
                                     // new client connected
        syslog(LOG_AUTHPRIV | LOG_DEBUG, "New incoming client");
        struct connection* newClient = secAlloc(sizeof(struct connection));
        newClient->msgsock           = secAlloc(sizeof(int));
        *(newClient->msgsock)        = accept(*(listencon.sock), 0, 0);
        if (*(newClient->msgsock) >= 0) {
          syslog(LOG_AUTHPRIV | LOG_DEBUG, "accepted new client sock: %d",
                 *(newClient->msgsock));
          list_rpush(connections, list_node_new(newClient));
          syslog(LOG_AUTHPRIV | LOG_DEBUG, "updated client list");
        } else {
          syslog(LOG_AUTHPRIV | LOG_ERR, "%m");
        }
      }
      // Check all client sockets for new messages
      int j;
      for (j = connections->len - 1; j >= 0; j--) {
        struct connection* con = list_at(connections, j)->val;
        syslog(LOG_AUTHPRIV | LOG_DEBUG, "Checking client %d", *(con->msgsock));
        if (FD_ISSET(*(con->msgsock), &readSockSet)) {
          syslog(LOG_AUTHPRIV | LOG_DEBUG, "New message for read av");
          return con;
        }
      }
    } else if (ret == 0) {  // might have reached a timeout, but not necessarily
                            // if (death < time(NULL)) {  // reached timeout
      syslog(LOG_AUTHPRIV | LOG_DEBUG, "Reached select timeout");
      return NULL;
      // }
      // continue;
    } else {
      syslog(LOG_AUTHPRIV | LOG_ERR, "%m");
    }
  }
  return NULL;
}

char* ipc_communicateWithPath(char* fmt, ...) {
  va_list args;
  va_start(args, fmt);

  return ipc_vcommunicateWithPath(fmt, args);
}

char* ipc_vcommunicateWithPath(char* fmt, va_list args) {
  static struct connection con;
  if (ipc_initWithPath(&con) != OIDC_SUCCESS) {
    return NULL;
  }
  if (ipc_connect(con) < 0) {
    return NULL;
  }
  char* response = ipc_communicateWithSock(*(con.sock), fmt, args);
  ipc_closeConnection(&con);
  return response;
}

extern list_t* encryptionKeys;

oidc_error_t server_ipc_write(const int sock, char* fmt, ...) {
  va_list args;
  va_start(args, fmt);
  if (encryptionKeys == NULL || encryptionKeys->len <= 0) {
    return ipc_vwrite(sock, fmt, args);
  }
  list_node_t*       node = list_rpop(encryptionKeys);
  struct ipc_keySet* keys = node->val;
  LIST_FREE(node);

  oidc_error_t e = ipc_vcryptWrite(sock, keys->key_tx, fmt, args);
  secFree(keys);
  if (e == OIDC_SUCCESS) {
    return OIDC_SUCCESS;
  }
  return ipc_writeOidcErrno(sock);
}

char* server_ipc_read(const int sock) {
  char* msg = ipc_read(sock);
  if (isJSONObject(msg)) {
    return msg;
  }
  char* res = server_ipc_cryptRead(sock, msg);
  secFree(msg);
  if (res == NULL) {  // TODO might be the wrong place
    ipc_writeOidcErrno(sock);
  }
  return res;
}

void server_ipc_freeLastKey() {
  if (encryptionKeys == NULL || encryptionKeys->len <= 0) {
    return;
  }
  list_node_t*       node = list_rpop(encryptionKeys);
  struct ipc_keySet* keys = node->val;
  LIST_FREE(node);
  secFree(keys);
}

oidc_error_t server_ipc_writeOidcErrno(const int sock) {
  return server_ipc_write(sock, RESPONSE_ERROR, oidc_serror());
}
