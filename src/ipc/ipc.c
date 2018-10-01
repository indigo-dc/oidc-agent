#define _XOPEN_SOURCE 700

#include "ipc.h"
#include "../oidc_error.h"
#include "../utils/cleaner.h"

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <unistd.h>
#include <syslog.h>

#include <sys/un.h>
#include <sys/ioctl.h>
#include <sys/fcntl.h>
#include <sys/socket.h>
#include <sys/select.h>

#define SOCKET_DIR "/tmp/oidc-XXXXXX"

char* dir = NULL;

/** @fn char* init_socket_path(const char* env_var_name)
 * @brief generates the socket path and prints commands for setting env vars
 * @param env_var_name the name of the environment variable which will be set.
 * If NULL non will be set.
 * @return a pointer to the socket_path. Has to be freed after usage.
 */
char* init_socket_path(const char* env_var_name) {
  if(NULL==dir) {
    dir = calloc(sizeof(char), strlen(SOCKET_DIR)+1);
    strcpy(dir, SOCKET_DIR);
    if(mkdtemp(dir)==NULL) {
      syslog(LOG_AUTHPRIV|LOG_ALERT, "%m");
      oidc_errno = OIDC_EMKTMP;
      return NULL;
    }
  }
  pid_t ppid = getppid();
  char* prefix = "oidc-agent";
  char* fmt = "%s/%s.%d";
  char* socket_path = oidc_sprintf(fmt, dir, prefix, ppid);
  if(env_var_name) {
    // printf("You have to set env var '%s' to '%s'. Please use the following statement:\n", env_var_name, socket_path);
    printf("%s=%s; export %s;\n", env_var_name, socket_path, env_var_name);
  }
  return socket_path;
}

oidc_error_t initConnectionWithoutPath(struct connection* con, int isServer) {
  con->server = calloc(sizeof(struct sockaddr_un),1);
  con->sock = calloc(sizeof(int),1);
  if(isServer) {
    con->msgsock = calloc(sizeof(int),1); // msgsock is not needed for a client; furthermore if the client calls ipc_close it would close stdin
  }
  if(con->server==NULL || con->sock==NULL || (con->msgsock==NULL && isServer)) {
    syslog(LOG_AUTHPRIV|LOG_ALERT, "alloc failed\n");
    exit(EXIT_FAILURE);
  }

  *(con->sock) = socket(AF_UNIX, SOCK_SEQPACKET, 0);
  if(*(con->sock) < 0) {
    syslog(LOG_AUTHPRIV|LOG_ERR, "opening stream socket: %m");
    oidc_errno = OIDC_ECRSOCK;
    return oidc_errno;
  }
  con->server->sun_family = AF_UNIX;
  return OIDC_SUCCESS;
}

/** @fn int ipc_init(struct connection* con, const char* env_var_name, int isServer)
 * @brief initializes unix domain socket
 * @param con, a pointer to the connection struct. The relevant fields will be
 * initialized.
 * @param env_var_name, the socket_path environment variable name @see
 * init_socket_path
 * @param isServer, specifies if the function is called from a server or client
 * @return 0 on success, otherwise a negative error code
 */
oidc_error_t ipc_init(struct connection* con, const char* env_var_name, int isServer) {
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "initializing ipc\n");
  if(initConnectionWithoutPath(con, isServer)!=OIDC_SUCCESS) {
    return oidc_errno;
  } 
  if(isServer) {
    char* path = init_socket_path(env_var_name);
    if(path == NULL) {
      return oidc_errno;
    }
    strcpy(con->server->sun_path, path);
    clearFreeString(path);
    server_socket_path = con->server->sun_path; 
  } else {
    char* path = getenv(env_var_name);
    if(path==NULL) {
      printError("Could not get the socket path from env var '%s'. Have you started oidc-agent and set the env var?\n", env_var_name);
      syslog(LOG_AUTHPRIV|LOG_WARNING, "Could not get the socket path from env var '%s'", env_var_name);
      oidc_errno = OIDC_EENVVAR;
      return OIDC_EENVVAR;
    } else {
      strcpy(con->server->sun_path, path); 
    }

  }
  return OIDC_SUCCESS;
}

/** @fn int ipc_initWithPath(struct connection* con)
 * @brief initializes unix domain socket with the current server_socket_path
 * @param con, a pointer to the connection struct. The relevant fields will be
 * initialized.
 * @return 0 on success, otherwise a negative error code
 */
oidc_error_t ipc_initWithPath(struct connection* con) {
  if(server_socket_path==NULL) {
    oidc_setArgNullFuncError(__func__);
    return oidc_errno;
  }
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "initializing ipc with path %s\n", server_socket_path);
  if(initConnectionWithoutPath(con, 0)!=OIDC_SUCCESS) {
    return oidc_errno;
  }
  strcpy(con->server->sun_path, server_socket_path); 
  return OIDC_SUCCESS;
}

/** @fn int ipc_bindAndListen(struct connection con)
 * @brief binds the server socket and listen 
 * @param con, the connection struct
 * @return 0 on success or errorcode on failure
 */
int ipc_bindAndListen(struct connection* con) {
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "binding ipc\n");
  unlink(con->server->sun_path);
  if(bind(*(con->sock), (struct sockaddr *) con->server, sizeof(struct sockaddr_un))) {
    syslog(LOG_AUTHPRIV|LOG_ALERT, "binding stream socket: %m");
    close(*(con->sock));
    oidc_errno = OIDC_EBIND;
    return OIDC_EBIND;
  }
  int flags;
  if(-1 == (flags = fcntl(*(con->sock), F_GETFL, 0)))
    flags = 0;
  fcntl(*(con->sock), F_SETFL, flags | O_NONBLOCK);

  syslog(LOG_AUTHPRIV|LOG_DEBUG, "listen ipc\n");
  return listen(*(con->sock), 5);
}


/** @fn int ipc_connect(struct connection con)
 * @brief connects to a UNIX Domain socket
 * @param con, the connection struct
 * @return the socket or OIDC_ECONSOCK on failure
 */
int ipc_connect(struct connection con) {
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "connecting ipc %s\n", con.server->sun_path);
  if(connect(*(con.sock), (struct sockaddr *) con.server, sizeof(struct sockaddr_un)) < 0) {
    close(*(con.sock));
    syslog(LOG_AUTHPRIV|LOG_ERR, "connecting stream socket: %m");
    oidc_errno = OIDC_ECONSOCK;
    return OIDC_ECONSOCK;
  }
  return *(con.sock);
}

/** @fn char* ipc_read(int _sock)
 * @brief reads from a socket
 * @param _sock the socket to read from
 * @return a pointer to the readed content. Has to be freed after usage. If NULL
 * is returned, it's most likely that the other party disconnected.
 */
char* ipc_read(int _sock) {
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "ipc reading from socket %d\n",_sock);
  if(_sock < 0) {
    syslog(LOG_AUTHPRIV|LOG_ERR, "invalid socket in ipc_read");
    oidc_errno = OIDC_ESOCKINV;
    return NULL;
  } else {
    int len = 0;
    int rv;
    fd_set set;
    FD_ZERO(&set); 
    FD_SET(_sock, &set); 
    rv = select(_sock + 1, &set, NULL, NULL, NULL);
    if(rv > 0) {
      if(ioctl(_sock, FIONREAD, &len)!=0) {
        syslog(LOG_AUTHPRIV|LOG_ERR, "ioctl: %m");
        oidc_errno = OIDC_EIOCTL;
        return NULL;
      }
      if(len > 0) {
        char* buf = calloc(sizeof(char), len+1);
        len = read(_sock, buf, len);
        syslog(LOG_AUTHPRIV|LOG_DEBUG, "ipc read %s\n",buf);
        return buf;
      } else {
        syslog(LOG_AUTHPRIV|LOG_DEBUG, "Client disconnected");
        oidc_errno = OIDC_EIPCDIS;
        return NULL;
      }
    } else if (rv == -1) {
      syslog(LOG_AUTHPRIV|LOG_ALERT, "error select in ipc_read: %m");
      oidc_errno = OIDC_ESELECT;
      return NULL;
    } else if (rv == 0) {
      // timeout can not happen with infinite timeout
    }
    oidc_errno = OIDC_EERROR;
    return NULL;
  }
}

/** @fn int ipc_write(int _sock, char* msg)
 * @brief writes a message to a socket
 * @param _sock the socket to write to
 * @param msg the msg to be written
 * @return 0 on success; -1 on failure
 */
oidc_error_t ipc_write(int _sock, char* fmt, ...) {
  va_list args;
  va_start(args, fmt);
  return ipc_vwrite(_sock, fmt, args);
  }

oidc_error_t ipc_vwrite(int _sock, char* fmt, va_list args) {
  va_list original;
  va_copy(original, args);
  char* msg = calloc(sizeof(char), vsnprintf(NULL, 0, fmt, args)+1);
  vsprintf(msg, fmt, original);
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "ipc writing to socket %d\n",_sock);
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "ipc write %s\n",msg);
  if(write(_sock, msg, strlen(msg)) < 0) {
    syslog(LOG_AUTHPRIV|LOG_ALERT, "writing on stream socket: %m");
    clearFreeString(msg);
    oidc_errno = OIDC_EWRITE;
    return oidc_errno;
  }
  clearFreeString(msg);
  return OIDC_SUCCESS;
}

oidc_error_t ipc_writeOidcErrno(int sock) {
  return ipc_write(sock, RESPONSE_ERROR, oidc_serror());
}

/** @fn int ipc_close(struct connection con)
 * @brief closes an ipc connection
 * @param con, the connection struct
 * @return 0 on success
 */
oidc_error_t ipc_close(struct connection* con) {
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "close ipc\n");
  if(con->sock!=NULL) {
    close(*(con->sock));
  }
  if(con->msgsock!=NULL) {
    close(*(con->msgsock));
  }
  clearFree(con->server, sizeof(*(con->server))); con->server = NULL;
  clearFree(con->sock, sizeof(*(con->sock))); con->sock = NULL;
  clearFree(con->msgsock, sizeof(*(con->msgsock))); con->msgsock = NULL;
  return OIDC_SUCCESS;
}

/** @fn int ipc_closeAndUnlink(struct connection con)
 * @brief closes an ipc connectiona and removes the socket
 * @param con, the connection struct
 * @return 0 on success
 */
oidc_error_t ipc_closeAndUnlink(struct connection* con) {
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "Unlinking %s", con->server->sun_path);
  unlink(con->server->sun_path);
  ipc_close(con);
  return OIDC_SUCCESS;
}

