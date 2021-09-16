#ifndef __APPLE__
#define _XOPEN_SOURCE 700
#endif
#include "serveripc.h"

#include <string.h>
#include <sys/fcntl.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <time.h>
#include <unistd.h>

#include "cryptIpc.h"
#include "defines/ipc_values.h"
#include "ipc.h"
#include "ipc/cryptCommunicator.h"
#include "utils/db/connection_db.h"
#include "utils/file_io/fileUtils.h"
#include "utils/file_io/file_io.h"
#include "utils/json.h"
#include "utils/logger.h"
#include "utils/memory.h"
#include "utils/string/stringUtils.h"
#include "wrapper/list.h"

#define SOCKET_TMP_DIR "/tmp"
#define SOCKET_DIR_PATTERN "oidc-XXXXXX"

#define TMPDIR_ENVVAR "TMPDIR"

static char* oidc_ipc_dir       = NULL;
static char* server_socket_path = NULL;

char* get_socket_dir_pattern() {
  const char* tmpdir = getenv(TMPDIR_ENVVAR);
  if (!tmpdir || !tmpdir[0]) {
    tmpdir = SOCKET_TMP_DIR;
  }
  return oidc_pathcat(tmpdir, SOCKET_DIR_PATTERN);
}

char* concat_default_socket_name_to_socket_path() {
  if (oidc_ipc_dir == NULL) {
    return NULL;
  }
  pid_t             ppid   = getppid();
  const char* const prefix = "oidc-agent";
  return oidc_sprintf("%s%s%s.%d", oidc_ipc_dir,
                      lastChar(oidc_ipc_dir) == '/' ? "" : "/", prefix, ppid);
}

char* create_new_socket_path() {
  if (NULL == oidc_ipc_dir) {
    oidc_ipc_dir = get_socket_dir_pattern();
    if (mkdtemp(oidc_ipc_dir) == NULL) {
      logger(ALERT, "%m");
      oidc_errno = OIDC_EMKTMP;
      secFree(oidc_ipc_dir);
      return NULL;
    }
  }
  return concat_default_socket_name_to_socket_path();
}

#define mkpath_return_on_error(p)        \
  if (mkpath(p, 0700) != OIDC_SUCCESS) { \
    secFree(oidc_ipc_dir);               \
    secFree(socket_file);                \
    return NULL;                         \
  }

oidc_error_t create_pre_random_part_path(const int rand_index) {
  *(oidc_ipc_dir + rand_index) = '\0';
  char* startRandomPart        = strrchr(oidc_ipc_dir, '/');
  if (startRandomPart && startRandomPart != oidc_ipc_dir) {
    *startRandomPart = '\0';
    if (mkpath(oidc_ipc_dir, 0700) != OIDC_SUCCESS) {
      *(oidc_ipc_dir + rand_index) = 'X';  // start of 'XXXXXX/' was an 'X'
      *startRandomPart             = '/';
      return oidc_errno;
    }
    *startRandomPart = '/';
  }
  *(oidc_ipc_dir + rand_index) = 'X';  // start of 'XXXXXX/' was an 'X'
  return OIDC_SUCCESS;
}

oidc_error_t create_random_part_path(const int rand_index) {
  *(oidc_ipc_dir + rand_index + 6) = '\0';
  if (mkdtemp(oidc_ipc_dir) == NULL) {
    logger(ERROR, "%s", oidc_ipc_dir);
    logger(ERROR, "mkdtemp: %m");
    oidc_errno                       = OIDC_EMKTMP;
    *(oidc_ipc_dir + rand_index + 6) = '/';
    return oidc_errno;
  }
  *(oidc_ipc_dir + rand_index + 6) = '/';
  return OIDC_SUCCESS;
}

oidc_error_t create_post_random_part_path(const int rand_index) {
  if (strlen(oidc_ipc_dir) - rand_index <= 7) {
    return OIDC_SUCCESS;
  }
  return mkpath(oidc_ipc_dir, 0700);
}

oidc_error_t create_path_with_random_part(const int rand_index) {
  oidc_error_t e = create_pre_random_part_path(rand_index);
  if (e != OIDC_SUCCESS) {
    return e;
  }
  e = create_random_part_path(rand_index);
  if (e != OIDC_SUCCESS) {
    return e;
  }
  return create_post_random_part_path(rand_index);
}

char* create_passed_socket_path(const char* requested_path) {
  char* socket_file = NULL;
  oidc_ipc_dir      = oidc_strcopy(requested_path);
  if (lastChar(oidc_ipc_dir) == '/') {  // only dir specified
    lastChar(oidc_ipc_dir) = '\0';
  } else {  // full path including file specified
    char* lastSlash = strrchr(oidc_ipc_dir, '/');
    socket_file     = oidc_strcopy(lastSlash + 1);
    char* tmp       = oidc_strncopy(oidc_ipc_dir, lastSlash - oidc_ipc_dir);
    secFree(oidc_ipc_dir);
    oidc_ipc_dir = tmp;
  }
  char* random_part = strstr(oidc_ipc_dir, "XXXXXX/")
                          ?: strEnds(oidc_ipc_dir, "XXXXXX")
                             ? oidc_ipc_dir + strlen(oidc_ipc_dir) - 6
                             : NULL;
  if (random_part == NULL) {
    mkpath_return_on_error(oidc_ipc_dir);
  } else {
    if (create_path_with_random_part(random_part - oidc_ipc_dir) !=
        OIDC_SUCCESS) {
      secFree(oidc_ipc_dir);
      secFree(socket_file);
      return NULL;
    }
  }
  if (socket_file == NULL) {
    return concat_default_socket_name_to_socket_path();
  }
  char* socket_path = oidc_pathcat(oidc_ipc_dir, socket_file);
  secFree(socket_file);
  return socket_path;
}

oidc_error_t socket_apply_group(const char* group_name) {
  if (group_name == NULL) {
    return OIDC_SUCCESS;
  }
  return changeGroup(oidc_ipc_dir, group_name);
}

/**
 * @brief generates the socket path and prints commands for setting env vars
 * @param group_name if not @c NULL, the group ownership is adjusted to the
 * specified group after creation.
 * @param socket_path if not @c NULL, the socket will be created at this path.
 * @return a pointer to the socket_path. Has to be freed after usage.
 */
char* init_socket_path(const char* group_name, const char* socket_path) {
  char* created_path = socket_path ? create_passed_socket_path(socket_path)
                                   : create_new_socket_path();
  if (created_path) {
    if (socket_apply_group(group_name) != OIDC_SUCCESS) {
      secFree(created_path);  // also sets created_path=NULL
    }
  }
  return created_path;
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
oidc_error_t ipc_server_init(struct connection* con, const char* group_name,
                             const char* socket_path) {
  logger(DEBUG, "initializing server ipc");
  if (initServerConnection(con) != OIDC_SUCCESS) {
    return oidc_errno;
  }
  char* path = init_socket_path(group_name, socket_path);
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
    secFree(timeout);
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
  if (msg == NULL || isJSONObject(msg)) {
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
