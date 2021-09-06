#include "ipc.h"
#include "defines/ipc_values.h"
#include "defines/settings.h"
#include "utils/ipUtils.h"
#include "utils/logger.h"
#include "utils/memory.h"
#include "utils/oidc_error.h"
#include "utils/printer.h"
#include "utils/stringUtils.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <winsock2.h>
#include <process.h>

#include "utils/registryConnector.h"
#include "utils/file_io/file_io.h"



/**
 * @brief initializes a tcp socket on the loopback address (MSYS emulated)
 * @param con, a pointer to the connection struct. The relevant fields will be
 * initialized.
 * @param remote, determines whether its remote communication or not (remote communication currently not supported for Windows)
 */
oidc_error_t ipc_client_init(struct connection* con, unsigned char remote) {
  if (remote) {
    logger(DEBUG, "Remote connections are currently not supported by windows oidc-agent library");
    return OIDC_EERROR;
  }
	con->msys_server = secAlloc(sizeof(struct sockaddr_in));
	con->sock = secAlloc(sizeof(SOCKET));

  // Extract port and authorization string from MSYS socket file
  char *socketpath = getRegistryValue(OIDC_SOCK_ENV_NAME);
  int port;
  sscanf(readFile(socketpath), "!<socket >%d s %X-%X-%X-%X", &port, &con->msys_secret[0], &con->msys_secret[1], &con->msys_secret[2], &con->msys_secret[3]);

	con->msys_server->sin_port = htons(port);
	con->msys_server->sin_addr.s_addr = inet_addr(SOCKET_LOOPBACK_ADDRESS);
	con->msys_server->sin_family = AF_INET;

	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2,2),&wsa) != 0)
	{
		return OIDC_ECONSOCK;
	}

	if((*(con->sock) = socket(AF_INET , SOCK_STREAM , 0 )) == INVALID_SOCKET)
	{
		return OIDC_ECRSOCK;
	}

	return OIDC_SUCCESS;
}

/**
 * @brief connects to the TCP socket on the loopback address (local commincation with MSYS emulated sockets)
 * @param con, the connection struct
 * @return @c OIDC_SUCCESS or @c OIDC_ECONSOCK on failure
 */
oidc_error_t ipc_connect(struct connection con) {
	if (connect((SOCKET)*(con.sock), (struct sockaddr *)con.msys_server , sizeof(struct sockaddr_in)) < 0)
	{
    closesocket(*(con.sock));
    WSACleanup();
    oidc_errno = OIDC_ECONSOCK;
		return oidc_errno;
	}
  return OIDC_SUCCESS;
}

/**
 * @brief authorizes against msys emulated socket
 * @param con, the connection struct
 * @return @c OIDC_SUCCESS or @c OIDC_EMSYSAUTH on failure
 */
oidc_error_t ipc_msys_authorize(struct connection con) {
    char *ptr = (char *) con.msys_secret;
    int written_bytes = send(*(con.sock), ptr , sizeof con.msys_secret , 0);
    if (written_bytes != sizeof con.msys_secret) {
        oidc_errno = OIDC_EMSYSAUTH;
		    return oidc_errno;
    }
    
    int out[4] = { 0, 0, 0, 0};
    ptr = (char *) out;
    int recv_size = recv(*(con.sock), ptr , sizeof out , 0);
    if (recv_size != sizeof out) {
        oidc_errno = OIDC_EMSYSAUTH;
		    return oidc_errno;
    }

    int messageCred[3] = {_getpid(), -1, -1};
    ptr = (char *) messageCred;
    written_bytes = send(*(con.sock) , ptr , sizeof messageCred , 0);
    if (written_bytes != sizeof messageCred) {
        oidc_errno = OIDC_EMSYSAUTH;
		    return oidc_errno;
    }
    
    int outCred[3] = {0, 0, 0};
    ptr = (char *) outCred;
    recv_size = recv(*(con.sock) , ptr, sizeof outCred , 0);
    if (recv_size != sizeof outCred) {
        oidc_errno = OIDC_EMSYSAUTH;
		    return oidc_errno;
    }
    
  	return OIDC_SUCCESS;
}

char* ipc_read(const SOCKET _sock) { 
    logger(DEBUG, "ipc reading from socket %d\n", _sock);

    u_long len = 0;
    int rv;
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
    while (read_bytes < (int) len) {
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
}

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
  int msg_len = strlen(msg);
  if (msg_len == 0) {  // Don't send an empty message. This will be read as
                       // client disconnected
    msg_len = 1;
    secFree(msg);
    msg = oidc_strcopy(" ");
  }
  logger(DEBUG, "ipc write message '%s'", msg);
  int written_bytes = send(_sock, msg, msg_len, 0);
  secFree(msg);
  if (written_bytes < 0) {
    logger(ALERT, "writing on stream socket: %m");
    oidc_errno = OIDC_EWRITE;
    return oidc_errno;
  }
  if (written_bytes < msg_len) {
    oidc_errno = OIDC_EMSGSIZE;
    return oidc_errno;
  }
  return OIDC_SUCCESS;
}

char* ipc_vcommunicateWithSock(SOCKET sock, const char* fmt, va_list args) {
  ipc_write(sock, fmt, args);
  return ipc_read(sock);
}

char* ipc_communicateWithSock(SOCKET sock, const char* fmt, ...) {
  va_list args;
  va_start(args, fmt);
  char* ret = ipc_vcommunicateWithSock(sock, fmt, args);
  va_end(args);
  return ret;
}

int ipc_close(SOCKET _sock) { 
	WSACleanup();
	return closesocket(_sock); 
}

oidc_error_t ipc_closeConnection(struct connection* con) {
  logger(DEBUG, "close ipc\n");
  if (con->sock != NULL) {
    ipc_close(*(con->sock));
  }
  secFree(con->msys_server);
  con->msys_server = NULL;
  secFree(con->sock);
  con->sock = NULL;
  return OIDC_SUCCESS;
}