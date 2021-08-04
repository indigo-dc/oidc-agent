#include "ipc.h"

#include <arpa/inet.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include "defines/ipc_values.h"
#include "defines/settings.h"
#include "utils/ipUtils.h"
#include "utils/logger.h"
#include "utils/memory.h"
#include "utils/oidc_error.h"
#include "utils/string/stringUtils.h"

oidc_error_t initConnectionWithoutPath(struct connection* con, int isServer,
                                       int tcp) {
  con->server     = secAlloc(sizeof(struct sockaddr_un));
  con->tcp_server = secAlloc(sizeof(struct sockaddr_in));
  con->sock       = secAlloc(sizeof(int));
  if (isServer) {  // msgsock is not needed for a client; furthermore if
                   // the client calls ipc_close it would close stdin
    con->msgsock = secAlloc(sizeof(int));
  }
  if (con->server == NULL || con->sock == NULL ||
      (con->msgsock == NULL && isServer)) {
    logger(ALERT, "alloc failed\n");
    return oidc_errno;
  }

  *(con->sock) = socket(tcp ? AF_INET : AF_UNIX, SOCK_STREAM, 0);
  if (*(con->sock) < 0) {
    logger(ERROR, "opening stream socket: %m");
    oidc_errno = OIDC_ECRSOCK;
    return oidc_errno;
  }
  con->server->sun_family     = AF_UNIX;
  con->tcp_server->sin_family = AF_INET;
  return OIDC_SUCCESS;
}

oidc_error_t initClientConnection(struct connection* con, int tcp) {
  return initConnectionWithoutPath(con, 0, tcp);
}

oidc_error_t initConnectionWithPath(struct connection* con,
                                    const char*        socket_path) {
  logger(DEBUG, "initializing ipc with path %s\n", socket_path);
  if (initConnectionWithoutPath(con, 0, 0) != OIDC_SUCCESS) {
    return oidc_errno;
  }
  strcpy(con->server->sun_path, socket_path);
  return OIDC_SUCCESS;
}

/**
 * @brief initializes a client unix domain or tcp socket
 * @param con, a pointer to the connection struct. The relevant fields will be
 * initialized.
 * @param env_var_name, the socket_path environment variable name
 */
oidc_error_t ipc_client_init(struct connection* con, unsigned char remote) {
  logger(DEBUG, "initializing client ipc");
  const char* env_var_name =
      remote ? OIDC_REMOTE_SOCK_ENV_NAME : OIDC_SOCK_ENV_NAME;
  const char* path = getenv(env_var_name);
  if (path == NULL) {
    char* err = oidc_sprintf("Could not get the socket path from env var '%s'. "
                             "Have you set the env var?\n",
                             env_var_name);
    logger(WARNING, "Could not get the socket path from env var '%s'",
           env_var_name);
    oidc_seterror(err);
    secFree(err);
    oidc_errno = OIDC_EENVVAR;
    return oidc_errno;
  }

  if (initClientConnection(con, remote) != OIDC_SUCCESS) {
    return oidc_errno;
  }

  if (remote) {
    logger(DEBUG, "Using TCP socket");
    char*          tmp_path   = oidc_strcopy(path);
    char*          ip         = strtok(tmp_path, ":");
    char*          port_str   = strtok(NULL, ":");
    unsigned short port       = port_str == NULL ? 0 : strToUShort(port_str);
    con->tcp_server->sin_port = htons(port ?: 42424);
    con->tcp_server->sin_addr.s_addr =
        inet_addr(isValidIP(ip) ? ip : hostnameToIP(ip));
    secFree(tmp_path);
  } else {
    logger(DEBUG, "Using UNIX domain socket");
    strcpy(con->server->sun_path, path);
  }
  return OIDC_SUCCESS;
}

/**
 * @brief connects to a UNIX Domain or TCP socket
 * @param con, the connection struct
 * @return the socket or @c OIDC_ECONSOCK on failure
 */
int ipc_connect(struct connection con) {
  struct sockaddr* server      = (struct sockaddr*)con.server;
  size_t           server_size = sizeof(struct sockaddr_un);
  if (con.server->sun_path[0] == '\0') {
    server      = (struct sockaddr*)con.tcp_server;
    server_size = sizeof(struct sockaddr_in);
    logger(DEBUG, "connecting tcp ipc %lu:%hu\n",
           con.tcp_server->sin_addr.s_addr, con.tcp_server->sin_port);
  } else {
    logger(DEBUG, "connecting ipc '%s'\n", con.server->sun_path);
  }
  if (connect(*(con.sock), server, server_size) < 0) {
    close(*(con.sock));
    logger(ERROR, "connecting stream socket: %m");
    oidc_errno = OIDC_ECONSOCK;
    return OIDC_ECONSOCK;
  }
  return *(con.sock);
}

/**
 * @brief reads from a socket until a timeout is reached
 * @param _sock the socket to read from
 * @param timeout the timeout in seconds, if @c 0 no timeout is used.
 * @return a pointer to the readed content. Has to be freed after usage. If
 * @c NULL is returned, it's most likely that the other party disconnected.
 */
char* ipc_read(const int _sock) { return ipc_readWithTimeout(_sock, 0); }

struct timeval* initTimeout(time_t death) {
  if (death == 0) {
    oidc_errno = OIDC_SUCCESS;
    return NULL;
  }
  time_t now = time(NULL);
  if (death < now) {
    logger(NOTICE, "death was before now");
    oidc_errno = OIDC_ETIMEOUT;
    return NULL;
  }
  struct timeval* timeout = secAlloc(sizeof(struct timeval));
  timeout->tv_sec         = death - now;
  oidc_errno              = OIDC_SUCCESS;
  return timeout;
}

/**
 * @brief reads from a socket until a timeout is reached
 * @param _sock the socket to read from
 * @param timeout the timeout in seconds, if @c 0 no timeout is used.
 * @return a pointer to the readed content. Has to be freed after usage. If an
 * error occurs or the timeout is reached @c NULL is returned and @c oidc_errno
 * is set.
 */
char* ipc_readWithTimeout(const int _sock, time_t death) {
  logger(DEBUG, "ipc reading from socket %d\n", _sock);
  if (_sock < 0) {
    logger(ERROR, "invalid socket in ipc_read");
    oidc_errno = OIDC_ESOCKINV;
    return NULL;
  }
  int    len = 0;
  int    rv;
  fd_set set;
  FD_ZERO(&set);
  FD_SET(_sock, &set);
  struct timeval* timeout = initTimeout(death);
  if (oidc_errno != OIDC_SUCCESS) {  // death before now
    return NULL;
  }
  rv = select(_sock + 1, &set, NULL, NULL, timeout);
  if (rv == -1) {
    logger(ALERT, "error select in %s: %m", __func__);
    oidc_errno = OIDC_ESELECT;
    return NULL;
  }
  if (rv == 0) {
    oidc_errno = OIDC_ETIMEOUT;
    return NULL;
  }
  if (ioctl(_sock, FIONREAD, &len) != 0) {
    logger(ERROR, "ioctl: %m");
    oidc_errno = OIDC_EIOCTL;
    return NULL;
  }
  if (len <= 0) {
    logger(DEBUG, "Client disconnected");
    oidc_errno = OIDC_EIPCDIS;
    return NULL;
  }
  char* buf = secAlloc(sizeof(char) * (len + 1));
  logger(DEBUG, "ipc want to read %d bytes", len);
  int read_bytes = 0;
  while (read_bytes < len) {
    int read_ret = read(_sock, buf + read_bytes, len - read_bytes);
    if (read_ret < 0) {
      oidc_setErrnoError();
      secFree(buf);
      return NULL;
    }
    read_bytes += read_ret;
    logger(DEBUG, "ipc did read %d bytes in total", read_bytes);
  }
  logger(DEBUG, "ipc read '%s'", buf);
  return buf;
}

/**
 * @brief writes a message to a socket
 * @param _sock the socket to write to
 * @param msg the msg to be written
 * @return @c 0 on success; on failure an error code is returned
 */
oidc_error_t ipc_write(int _sock, const char* fmt, ...) {
  va_list args;
  va_start(args, fmt);
  oidc_error_t ret = ipc_vwrite(_sock, fmt, args);
  va_end(args);
  return ret;
}

oidc_error_t ipc_vwrite(int _sock, const char* fmt, va_list args) {
  char* msg = oidc_vsprintf(fmt, args);
  if (msg == NULL) {
    return oidc_errno;
  }
  size_t msg_len = strlen(msg);
  if (msg_len == 0) {  // Don't send an empty message. This will be read as
                       // client disconnected
    msg_len = 1;
    secFree(msg);
    msg = oidc_strcopy(" ");
  }
  logger(DEBUG, "ipc writing %lu bytes to socket %d", msg_len, _sock);
  logger(DEBUG, "ipc write message '%s'", msg);
  ssize_t written_bytes = write(_sock, msg, msg_len);
  secFree(msg);
  if (written_bytes < 0) {
    logger(ALERT, "writing on stream socket: %m");
    oidc_errno = OIDC_EWRITE;
    return oidc_errno;
  }
  if ((size_t)written_bytes < msg_len) {
    oidc_errno = OIDC_EMSGSIZE;
    return oidc_errno;
  }
  return OIDC_SUCCESS;
}

oidc_error_t ipc_writeOidcErrno(int sock) {
  return ipc_write(sock, RESPONSE_ERROR, oidc_serror());
}

/**
 * @brief closes a FD
 * @param _sock the FD to be closed
 */
int ipc_close(int _sock) { return close(_sock); }

/**
 * @brief closes an ipc connection
 * @param con, a pointer to the connection struct
 * @return @c OIDC_SUCCESS on success
 */
oidc_error_t ipc_closeConnection(struct connection* con) {
  logger(DEBUG, "close ipc\n");
  if (con->sock != NULL) {
    ipc_close(*(con->sock));
  }
  if (con->msgsock != NULL) {
    ipc_close(*(con->msgsock));
  }
  secFree(con->server);
  con->server = NULL;
  secFree(con->tcp_server);
  con->tcp_server = NULL;
  secFree(con->sock);
  con->sock = NULL;
  secFree(con->msgsock);
  con->msgsock = NULL;
  return OIDC_SUCCESS;
}

/**
 * @brief closes an ipc connection and removes the socket
 * @param con, a pointer to the connection struct
 * @return @c OIDC_SUCCESS on success
 */
oidc_error_t ipc_closeAndUnlinkConnection(struct connection* con) {
  if (con->server->sun_path != NULL) {
    logger(DEBUG, "Unlinking %s", con->server->sun_path);
    unlink(con->server->sun_path);
  }
  ipc_closeConnection(con);
  return OIDC_SUCCESS;
}

char* ipc_vcommunicateWithSock(int sock, const char* fmt, va_list args) {
  ipc_vwrite(sock, fmt, args);
  return ipc_read(sock);
}

char* ipc_communicateWithSock(int sock, const char* fmt, ...) {
  va_list args;
  va_start(args, fmt);
  char* ret = ipc_vcommunicateWithSock(sock, fmt, args);
  va_end(args);
  return ret;
}
