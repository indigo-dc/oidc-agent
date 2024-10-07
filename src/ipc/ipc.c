#define _GNU_SOURCE
#include "ipc.h"

#include "defines/msys.h"
#ifdef MINGW
#include <process.h>
#include <winsock2.h>
#else
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#endif
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include "defines/ipc_values.h"
#include "defines/settings.h"
#include "utils/ipUtils.h"
#include "utils/logger.h"
#include "utils/memory.h"
#include "utils/oidc_error.h"
#include "utils/string/stringUtils.h"
#ifdef ANY_MSYS
#include "utils/file_io/file_io.h"
#include "utils/registryConnector.h"
#endif

#ifndef MINGW

char* defaultSocketPath() {
  const char* tmp = getenv("TMPDIR") ?: "/tmp";
  uid_t       uid = getuid();
  return oidc_sprintf("%s/oidc-agent-service-%d/oidc-agent.sock", tmp, uid);
}

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

#endif

/**
 * @brief initializes a client unix domain or tcp socket
 * @param con a pointer to the connection struct. The relevant fields will be
 * initialized.
 * @param remote determines whether its remote communication or not (remote
 * communication currently not supported for Windows)
 */
oidc_error_t ipc_client_init(struct connection* con, unsigned char remote) {
  logger(DEBUG, "initializing client ipc");
#ifdef MINGW
  if (remote) {
    logger(DEBUG, "Remote connections are currently not supported by windows "
                  "oidc-agent library");
    return OIDC_EERROR;
  }
  con->tcp_server = secAlloc(sizeof(struct sockaddr_in));
  con->sock       = secAlloc(sizeof(SOCKET));

  // Extract port and authorization string from MSYS socket file
  char* socketpath = getRegistryValue(OIDC_SOCK_ENV_NAME);
  int   port;
  sscanf(readFile(socketpath), "!<socket >%d s %X-%X-%X-%X", &port,
         &con->msys_secret[0], &con->msys_secret[1], &con->msys_secret[2],
         &con->msys_secret[3]);

  con->tcp_server->sin_port        = htons(port);
  con->tcp_server->sin_addr.s_addr = inet_addr(SOCKET_LOOPBACK_ADDRESS);
  con->tcp_server->sin_family      = AF_INET;

  WSADATA wsa;
  if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
    return OIDC_ECONSOCK;
  }

  if ((*(con->sock) = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
    return OIDC_ECRSOCK;
  }
  return OIDC_SUCCESS;
#else
  const char* env_var_name =
      remote ? OIDC_REMOTE_SOCK_ENV_NAME : OIDC_SOCK_ENV_NAME;
  unsigned char usedDefault = 0;
#ifdef ANY_MSYS
  char* path = getRegistryValue(env_var_name);
#else
  char* path = oidc_strcopy(getenv(env_var_name));
  if (path == NULL && remote == 0) {
    path        = defaultSocketPath();
    usedDefault = 1;
  }
#endif
  if (path == NULL) {
    char* err = oidc_sprintf(
        "Could not get the socket path from env var '%s'%s. "
        "Have you set the env var?\nIs a agent running?\n",
        env_var_name,
        usedDefault
            ? " and could not connect to default oidc-agent-service path"
            : "");
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
    char*          ip         = strtok(path, ":");
    char*          port_str   = strtok(NULL, ":");
    unsigned short port       = port_str == NULL ? 0 : strToUShort(port_str);
    con->tcp_server->sin_port = htons(port ?: 42424);
    con->tcp_server->sin_addr.s_addr =
        inet_addr(isValidIP(ip) ? ip : hostnameToIP(ip));
  } else {
    logger(DEBUG, "Using UNIX domain socket");
    strcpy(con->server->sun_path, path);
  }
  secFree(path);
  return OIDC_SUCCESS;
#endif
}

/**
 * @brief connects to a UNIX Domain or TCP socket
 * @param con the connection struct
 * @return @c OIDC_SUCCESS or @c OIDC_ECONSOCK on failure
 */
oidc_error_t ipc_connect(struct connection con) {
#ifdef MINGW
  if (connect(*(con.sock), (struct sockaddr*)con.tcp_server,
              sizeof(struct sockaddr_in)) < 0) {
    closesocket(*(con.sock));
    WSACleanup();
    oidc_errno = OIDC_ECONSOCK;
    return oidc_errno;
  }
  return OIDC_SUCCESS;
#else
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
    return oidc_errno;
  }
  return OIDC_SUCCESS;
#endif
}

#ifdef MINGW
/**
 * @brief authorizes against msys emulated socket
 * @param con, the connection struct
 * @return @c OIDC_SUCCESS or @c OIDC_EMSYSAUTH on failure
 */
oidc_error_t ipc_msys_authorize(struct connection con) {
  char* ptr           = (char*)con.msys_secret;
  int   written_bytes = send(*(con.sock), ptr, sizeof con.msys_secret, 0);
  if (written_bytes != sizeof con.msys_secret) {
    oidc_errno = OIDC_EMSYSAUTH;
    return oidc_errno;
  }

  int out[4]    = {0, 0, 0, 0};
  ptr           = (char*)out;
  int recv_size = recv(*(con.sock), ptr, sizeof out, 0);
  if (recv_size != sizeof out) {
    oidc_errno = OIDC_EMSYSAUTH;
    return oidc_errno;
  }

  int messageCred[3] = {_getpid(), -1, -1};
  ptr                = (char*)messageCred;
  written_bytes      = send(*(con.sock), ptr, sizeof messageCred, 0);
  if (written_bytes != sizeof messageCred) {
    oidc_errno = OIDC_EMSYSAUTH;
    return oidc_errno;
  }

  int outCred[3] = {0, 0, 0};
  ptr            = (char*)outCred;
  recv_size      = recv(*(con.sock), ptr, sizeof outCred, 0);
  if (recv_size != sizeof outCred) {
    oidc_errno = OIDC_EMSYSAUTH;
    return oidc_errno;
  }

  return OIDC_SUCCESS;
}
#endif

/**
 * @brief reads from a socket until a timeout is reached
 * @param _sock the socket to read from
 * @param timeout the timeout in seconds, if @c 0 no timeout is used.
 * @return a pointer to the readed content. Has to be freed after usage. If
 * @c NULL is returned, it's most likely that the other party disconnected.
 */
char* ipc_read(const SOCKET _sock) {
#ifdef MINGW
  logger(DEBUG, "ipc reading from socket %d\n", _sock);

  u_long len = 0;
  int    rv;
  fd_set set;
  FD_ZERO(&set);
  FD_SET(_sock, &set);

  rv = select(_sock + 1, &set, NULL, NULL, 0);
  if (rv == -1) {
    logger(ALERT, "error select in %s: %m", __func__);
    oidc_errno = OIDC_ESELECT;
    return NULL;
  }
  if (rv == 0) {
    oidc_errno = OIDC_ETIMEOUT;
    return NULL;
  }
  if (ioctlsocket(_sock, FIONREAD, &len) != 0) {
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
  while (read_bytes < (int)len) {
    int read_ret = recv(_sock, buf + read_bytes, len - read_bytes, 0);
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
#else
  return ipc_readWithTimeout(_sock, 0);
#endif
}

#ifndef MINGW
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
 * @param timeout the time when the request times out, if @c 0 no timeout is
 * used.
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
  secFree(timeout);
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
#ifdef __APPLE__
  // If we write large amount of data, i.e. >65536, this can be bigger than the
  // pipe's buffer. On linux we increase the buffer size, on windows it does not
  // seem to be a problem. On Macos, I could not find another solution, so we do
  // the following:
  // If we read exactly 65536 bytes, we try to read again (with a timeout of 1
  // second)
  if (read_bytes == 65536) {
    char* tmp = ipc_readWithTimeout(_sock, time(NULL) + 1);
    if (tmp != NULL) {
      char* b = oidc_strcat(buf, tmp);
      secFree(tmp);
      secFree(buf);
      buf = b;
    }
  }
#endif
  logger(DEBUG, "ipc read '%s'", buf);
  return buf;
}
#endif

/**
 * @brief writes a message to a socket
 * @param _sock the socket to write to
 * @param msg the msg to be written
 * @return @c 0 on success; on failure an error code is returned
 */
oidc_error_t ipc_write(SOCKET _sock, const char* fmt, ...) {
  va_list args;
  va_start(args, fmt);
  oidc_error_t ret = ipc_vwrite(_sock, fmt, args);
  va_end(args);
  return ret;
}

oidc_error_t ipc_vwrite(SOCKET _sock, const char* fmt, va_list args) {
  char* msg = oidc_vsprintf(fmt, args);
  if (msg == NULL) {
    return oidc_errno;
  }
#ifdef MINGW
  int msg_len = strlen(msg);
#else
  size_t msg_len = strlen(msg);
#endif
  if (msg_len == 0) {  // Don't send an empty message. This will be read as
                       // client disconnected
    msg_len = 1;
    secFree(msg);
    msg = oidc_strcopy(" ");
  }
  logger(DEBUG, "ipc writing %lu bytes to socket %d", msg_len, _sock);
  logger(DEBUG, "ipc write message '%s'", msg);
#ifdef MINGW
  int written_bytes = send(_sock, msg, msg_len, 0);
#else
#ifdef __linux__
  if (fcntl(_sock, F_GETPIPE_SZ) < (int)msg_len) {
    fcntl(_sock, F_SETPIPE_SZ, msg_len);
  }
#endif
  ssize_t written_bytes = write(_sock, msg, msg_len);
#endif
  secFree(msg);
  if (written_bytes < 0) {
    logger(ALERT, "writing on stream socket: %m");
    oidc_errno = OIDC_EWRITE;
    return oidc_errno;
  }
#ifdef MINGW
  if (written_bytes < msg_len) {
#else
  if ((size_t)written_bytes < msg_len) {
#endif
    oidc_errno = OIDC_EMSGSIZE;
    return oidc_errno;
  }
  return OIDC_SUCCESS;
}

oidc_error_t ipc_writeOidcErrno(SOCKET sock) {
  return ipc_write(sock, RESPONSE_ERROR, oidc_serror());
}

/**
 * @brief closes a FD
 * @param _sock the FD to be closed
 */
int ipc_close(SOCKET _sock) {
#ifdef MINGW
  WSACleanup();
  return closesocket(_sock);
#else
  return close(_sock);
#endif
}

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
#ifndef MINGW
  if (con->msgsock != NULL) {
    ipc_close(*(con->msgsock));
  }
  secFree(con->server);
  secFree(con->msgsock);
#endif
  secFree(con->sock);
  secFree(con->tcp_server);
  return OIDC_SUCCESS;
}

char* ipc_vcommunicateWithSock(SOCKET sock, const char* fmt, va_list args) {
  ipc_vwrite(sock, fmt, args);
  return ipc_read(sock);
}

char* ipc_communicateWithSock(SOCKET sock, const char* fmt, ...) {
  va_list args;
  va_start(args, fmt);
  char* ret = ipc_vcommunicateWithSock(sock, fmt, args);
  va_end(args);
  return ret;
}
