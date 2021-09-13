#ifndef __APPLE__
#define _XOPEN_SOURCE 700
#endif
#include "serveripc.h"
#include "cryptIpc.h"
#include "defines/ipc_values.h"
#include "ipc.h"
#include "ipc/cryptCommunicator.h"
#include "utils/db/connection_db.h"
#include "utils/file_io/fileUtils.h"
#include "utils/json.h"
#include "utils/logger.h"
#include "utils/memory.h"
#include "utils/printer.h"
#include "utils/stringUtils.h"
#include "wrapper/list.h"

#include <string.h>
#include <sys/fcntl.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <time.h>
#include <unistd.h>

#ifdef __MINGW32__
#include <windows.h>
#endif

#define SOCKET_DIR "/tmp/oidc-XXXXXX"

static char* oidc_ipc_dir       = NULL;
static char* server_socket_path = NULL;

/**
 * @brief generates the socket path and prints commands for setting env vars
 * @param env_var_name the name of the environment variable which will be set.
 * If NULL non will be set.
 * @param group_name if not @c NULL, the group ownership is adjusted to the
 * specified group after creation.
 * @return a pointer to the socket_path. Has to be freed after usage.
 */
char* init_socket_path(const char* group_name) {
  if (NULL == oidc_ipc_dir) {
    #ifdef __MINGW32__
    char currentPath[MAX_PATH];
    GetCurrentDirectory(MAX_PATH, currentPath);
    strcat(currentPath, SOCKET_DIR);
    oidc_ipc_dir = oidc_strcopy(currentPath);
    #else
    oidc_ipc_dir = oidc_strcopy(SOCKET_DIR);
    #endif
    if (mkdtemp(oidc_ipc_dir) == NULL) {
      logger(ALERT, "%m");
      oidc_errno = OIDC_EMKTMP;
      return NULL;
    }
    if (group_name != NULL) {
      if (changeGroup(oidc_ipc_dir, group_name) != OIDC_SUCCESS) {
        return NULL;
      }
    }
  }
  pid_t       ppid        = getppid();
  const char* prefix      = "oidc-agent";
  const char* fmt         = "%s/%s.%d";
  char*       socket_path = oidc_sprintf(fmt, oidc_ipc_dir, prefix, ppid);
  return socket_path;
}

oidc_error_t initServerConnection(struct connection* con) {
  return initConnectionWithoutPath(con, 1, 0);
}

/**
 * @brief initializes a server unix domain socket
 * @param con, a pointer to the connection struct. The relevant fields will be
 * initialized.
 * @param group_name if not @c NULL, the group ownership is adjusted to the
 * specified group after creation.
 */
oidc_error_t ipc_server_init(struct connection* con, const char* group_name) {
  logger(DEBUG, "initializing server ipc");
  if (initServerConnection(con) != OIDC_SUCCESS) {
    return oidc_errno;
  }
  char* path = init_socket_path(group_name);
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
  logger(DEBUG, "initializing ipc with path %s\n", server_socket_path);
  if (initConnectionWithoutPath(con, 0, 1) != OIDC_SUCCESS) {
    return oidc_errno;
  }
  strcpy(con->server->sun_path, server_socket_path);
  return OIDC_SUCCESS;
}

char* getServerSocketPath() { return server_socket_path; }

/**
 * @brief binds the server socket and starts listening
 * @param con, a pointer to the connection struct
 * @return @c 0 on success or an errorcode on failure
 */
int ipc_bindAndListen(struct connection* con) {
  logger(DEBUG, "binding ipc\n");
  unlink(con->server->sun_path);
  if (bind(*(con->sock), (struct sockaddr*)con->server,
           sizeof(struct sockaddr_un))) {
    logger(ALERT, "binding stream socket: %m");
    close(*(con->sock));
    oidc_errno = OIDC_EBIND;
    return OIDC_EBIND;
  }
  int flags;
  if (-1 == (flags = fcntl(*(con->sock), F_GETFL, 0)))
    flags = 0;
  fcntl(*(con->sock), F_SETFL, flags | O_NONBLOCK);

  logger(DEBUG, "listen ipc\n");
  return listen(*(con->sock), 5);
}

int _determineMaxSockAndAddToReadSet(int sock_listencon, fd_set* readSet) {
  int              maxSock = sock_listencon;
  list_node_t*     node;
  list_iterator_t* it = list_iterator_new(connectionDB_getList(), LIST_HEAD);
  while ((node = list_iterator_next(it))) {
    struct connection* con = node->val;
    FD_SET(*(con->msgsock), readSet);
    if (*(con->msgsock) > maxSock) {
      maxSock = *(con->msgsock);
    }
  }
  list_iterator_destroy(it);
  return maxSock;
}

struct connection* _checkClientSocksForMsg(fd_set* readSet) {
  list_node_t*     node;
  list_iterator_t* it = list_iterator_new(connectionDB_getList(), LIST_TAIL);
  while ((node = list_iterator_next(it))) {
    struct connection* con = node->val;
    logger(DEBUG, "Checking client %d", *(con->msgsock));
    if (FD_ISSET(*(con->msgsock), readSet)) {
      logger(DEBUG, "New message for read av");
      list_iterator_destroy(it);
      return con;
    }
  }
  list_iterator_destroy(it);
  return NULL;
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
 * @return A pointer to a client connection. On this connection is either a
 * message avaible for reading or the client disconnected.
 */
struct connection* ipc_readAsyncFromMultipleConnectionsWithTimeout(
    struct connection listencon, time_t death) {
  while (1) {
    fd_set readSockSet;
    FD_ZERO(&readSockSet);
    FD_SET(*(listencon.sock), &readSockSet);
    int maxSock =
        _determineMaxSockAndAddToReadSet(*(listencon.sock), &readSockSet);

    struct timeval* timeout = initTimeout(death);
    if (oidc_errno != OIDC_SUCCESS) {  // death before now
      return NULL;
    }
    logger(DEBUG, "Calling select with maxSock %d and timeout %lu", maxSock,
           timeout ? timeout->tv_sec : 0);
    // Waiting for incoming connections and messages
    int ret = select(maxSock + 1, &readSockSet, NULL, NULL, timeout);
    if (ret > 0) {
      if (FD_ISSET(*(listencon.sock),
                   &readSockSet)) {  // if listensock read something it means a
                                     // new client connected
        logger(DEBUG, "New incoming client");
        struct connection* newClient = secAlloc(sizeof(struct connection));
        newClient->msgsock           = secAlloc(sizeof(int));
        *(newClient->msgsock)        = accept(*(listencon.sock), 0, 0);
        if (*(newClient->msgsock) >= 0) {
          logger(DEBUG, "accepted new client sock: %d", *(newClient->msgsock));
          connectionDB_addValue(newClient);
          logger(DEBUG, "updated client list");
        } else {
          logger(ERROR, "%m");
        }
      }
      struct connection* con = _checkClientSocksForMsg(&readSockSet);
      if (con) {
        return con;
      }
    } else if (ret == 0) {
      logger(DEBUG, "Reached select timeout");
      oidc_errno = OIDC_ETIMEOUT;
      return NULL;
    } else {
      logger(ERROR, "%m");
    }
  }
  return NULL;
}

char* ipc_cryptCommunicateWithServerPath(const char* fmt, ...) {
  va_list args;
  va_start(args, fmt);
  char* ret = ipc_vcryptCommunicateWithServerPath(fmt, args);
  va_end(args);
  return ret;
}

char* ipc_vcryptCommunicateWithServerPath(const char* fmt, va_list args) {
  return ipc_vcryptCommunicateWithPath(server_socket_path, fmt, args);
}

extern list_t* encryptionKeys;

oidc_error_t server_ipc_write(const int sock, const char* fmt, ...) {
  va_list args;
  va_start(args, fmt);
  if (encryptionKeys == NULL || encryptionKeys->len <= 0) {
    oidc_error_t ret = ipc_vwrite(sock, fmt, args);
    va_end(args);
    return ret;
  }
  list_node_t*   node    = list_rpop(encryptionKeys);
  unsigned char* ipc_key = node->val;
  LIST_FREE(node);

  oidc_error_t e = ipc_vcryptWrite(sock, ipc_key, fmt, args);
  va_end(args);
  secFree(ipc_key);
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
  return res;
}

void server_ipc_freeLastKey() {
  if (encryptionKeys == NULL || encryptionKeys->len <= 0) {
    return;
  }
  list_node_t*   node = list_rpop(encryptionKeys);
  unsigned char* key  = node->val;
  LIST_FREE(node);
  secFree(key);
}

oidc_error_t server_ipc_writeOidcErrno(const int sock) {
  return server_ipc_write(sock, RESPONSE_ERROR, oidc_serror());
}

oidc_error_t server_ipc_writeOidcErrnoPlain(const int sock) {
  return ipc_writeOidcErrno(sock);
}
