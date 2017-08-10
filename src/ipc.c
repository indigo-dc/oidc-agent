#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <sys/fcntl.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>
#include <syslog.h>

#include "ipc.h"

#define SOCKET_DIR "/tmp/oidc-XXXXXX"

char* dir = NULL;

/** @fn char* init_socket_path(const char* prefix, const char* env_var_name)
 * @brief generates the socket path and sets the environment variable
 * @param prefix the prefix to be used, should descripe the usage of the socket
 * @param env_var_name the name of the environment variable which will be set.
 * If NULL non will be set.
 * @return a pointer to the socket_path. Has to be freed after usage.
 */
char* init_socket_path(const char* prefix, const char* env_var_name) {
  if(NULL==dir) {
    dir = calloc(sizeof(char), strlen(SOCKET_DIR)+1);
    strcpy(dir, SOCKET_DIR);
    if (mkdtemp(dir)==NULL) {
      syslog(LOG_AUTHPRIV|LOG_ALERT, "%m");
      return NULL;
    }
  }
  pid_t ppid = getppid();
  char* fmt = "%s/%s.%d";
  char* socket_path = calloc(sizeof(char), strlen(dir)+strlen(fmt)+strlen(prefix)+snprintf(NULL, 0, "%d", ppid)+1);
  sprintf(socket_path, fmt, dir, prefix, ppid);
  if(env_var_name) {
    printf("You have to set env var '%s' to '%s'. Please use the following statement:\n", env_var_name, socket_path);
    printf("%s=%s; export %s;\n", env_var_name, socket_path, env_var_name);
  }
  return socket_path;
}

/** @fn int ipc_init(struct connection* con, const char* prefix, const char* env_var_name, int isServer)
 * @brief initializes unix domain socket
 * @param con, a pointer to the connection struct. The relevant fields will be
 * initialized.
 * @param prefix, the prefix for the socket_path @see init_socket_path
 * @param env_var_name, the socket_path environment variable name @see
 * init_socket_path
 * @param isServer, specifies if the function is valled from a server or client
 * @return 0 on success, otherwise a negative error code
 */
int ipc_init(struct connection* con, const char* prefix, const char* env_var_name, int isServer) {
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "initializing ipc\n");
  con->server = calloc(sizeof(struct sockaddr_un),1);
  con->sock = calloc(sizeof(int),1);
  if (isServer)
    con->msgsock = calloc(sizeof(int),1); // msgsock is not needed for a client; also if the client calls ipc_close it would close stdin
  if(con->server==NULL || con->sock==NULL || (con->msgsock==NULL && isServer)) {
    syslog(LOG_AUTHPRIV|LOG_ALERT, "malloc failed\n");
    exit(EXIT_FAILURE);
  }

  *(con->sock) = socket(AF_UNIX, SOCK_STREAM, 0);
  if (*(con->sock) < 0) {
    syslog(LOG_AUTHPRIV|LOG_ERR, "opening stream socket: %m");
    return *(con->sock);
  }
  con->server->sun_family = AF_UNIX;

  if(isServer) {
    char* path = init_socket_path(prefix, env_var_name);
    strcpy(con->server->sun_path, path);
    free(path);
  } else {
    char* path = getenv(env_var_name);
    if(path==NULL) {
      printf("Could not get the socket path from env var '%s'. Have you started oidcd and set the env var?\n", env_var_name);
      return -1;
    } else {
      strcpy(con->server->sun_path, path); 
    }

  }
  return 0;
}

/** @fn int ipc_bind(struct connection con, void(callback)())
 * @brief binds the server socket, starts listening and accepting a connection
 * @param con, the connection struct
 * @param callback a callback function. It will be called between listen and
 * accepting a connection and can be used to start the communication partner
 * process. Can also be NULL.
 * @return the msgsock or -1 on failure
 */
int ipc_bind(struct connection* con) {
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "binding ipc\n");
  unlink(con->server->sun_path);
  if (bind(*(con->sock), (struct sockaddr *) con->server, sizeof(struct sockaddr_un))) {
    syslog(LOG_AUTHPRIV|LOG_ALERT, "binding stream socket: %m");
    close(*(con->sock));
    return -1;
  }
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "listen ipc\n");
  listen(*(con->sock), 5);
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "accepting ipc\n");
  *(con->msgsock) = accept(*(con->sock), 0, 0);
  return *(con->msgsock);
}

int ipc_bindAndListen(struct connection* con) {
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "binding ipc\n");
  unlink(con->server->sun_path);
  if (bind(*(con->sock), (struct sockaddr *) con->server, sizeof(struct sockaddr_un))) {
    syslog(LOG_AUTHPRIV|LOG_ALERT, "binding stream socket: %m");
    close(*(con->sock));
    return -1;
  }
  int flags;
  if (-1 == (flags = fcntl(*(con->sock), F_GETFL, 0)))
    flags = 0;
  fcntl(*(con->sock), F_SETFL, flags | O_NONBLOCK);

  syslog(LOG_AUTHPRIV|LOG_DEBUG, "listen ipc\n");
  return listen(*(con->sock), 5);

}

struct connection* ipc_async(struct connection listencon, struct connection** clientcons_addr, size_t* size) {

  //TODO test it, integrate it, so you can test it
  while(1){
    int maxSock = -1;
    fd_set readSockSet;
    FD_ZERO(&readSockSet);
    FD_SET(*(listencon.sock), &readSockSet);  
    if (*(listencon.sock) > maxSock) maxSock = *(listencon.sock);

    unsigned int i;
    for (i=0; i<*size; i++) {
      FD_SET(*((*clientcons_addr+i)->msgsock), &readSockSet);
      if (*((*clientcons_addr+i)->msgsock) > maxSock) maxSock = *((*clientcons_addr+i)->msgsock);
    }

    syslog(LOG_AUTHPRIV|LOG_DEBUG, "Selecting maxSock is %d", maxSock);
    int ret = select(maxSock+1, &readSockSet, NULL, NULL, NULL);
    if(ret >= 0) {
      if(FD_ISSET(*(listencon.sock), &readSockSet)) {
        syslog(LOG_AUTHPRIV|LOG_DEBUG, "New incoming client");
        struct connection newClient = {0, 0, 0};
        newClient.msgsock = calloc(sizeof(int), 1);
        *(newClient.msgsock) = accept(*(listencon.sock), 0, 0);
        if (*(newClient.msgsock) >= 0) {
          syslog(LOG_AUTHPRIV|LOG_DEBUG, "accepted new client sock: %d", *(newClient.msgsock));

          *clientcons_addr = addConnection(*clientcons_addr, size, newClient);
          syslog(LOG_AUTHPRIV|LOG_DEBUG, "updated client list");
        }
        else perror("accept");
      }

      int j;
      for (j=*size-1; j>=0; j--){
        syslog(LOG_AUTHPRIV|LOG_DEBUG, "Checking client %d", *((*clientcons_addr+j)->msgsock));
        if (FD_ISSET(*((*clientcons_addr+j)->msgsock), &readSockSet))
        {
          syslog(LOG_AUTHPRIV|LOG_DEBUG, "New message for read av");
          return *clientcons_addr+j;           }
      }
    }
    else perror("select");
  }
  return NULL;


  // int rv;
  // struct timeval timeout;
  // fd_set set;
  // timeout.tv_sec = timeout_s;
  // timeout.tv_usec = 0;
  // FD_ZERO(&set);
  // unsigned int i;
  // int maxfd = 0;
  // for(i=0; i<size; i++) {
  //   FD_SET(*((con+i)->sock), &set); 
  //   if(*((con+i)->sock)>maxfd)
  //     maxfd = *((con+i)->sock);
  // }
  // rv = select(maxfd + 1, &set, NULL, NULL, timeout_s<0 ? NULL : &timeout);
  // if(rv > 0) {
  //   *(con->msgsock) = accept(*(con->sock), 0, 0);
  //   return *(con->msgsock);
  // }
  // else if (rv == -1)
  //   syslog(LOG_AUTHPRIV|LOG_ALERT, "error select in ipc_accept_async: %m");
  // return rv;
}

/** @fn int ipc_connect(struct connection con)
 * @brief connects to a UNIX Domain socket
 * @param con, the connection struct
 * @return the socket or -1 on failure
 */
int ipc_connect(struct connection con) {
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "connecting ipc\n");
  if (connect(*(con.sock), (struct sockaddr *) con.server, sizeof(struct sockaddr_un)) < 0) {
    close(*(con.sock));
    syslog(LOG_AUTHPRIV|LOG_ERR, "connecting stream socket: %m");
    return -1;
  }
  return *(con.sock);
}

/** @fn 
 * char* ipc_read(int _sock)
 * @brief reads from a socket
 * @param _sock the socket to read from
 * @return a pointer to the read content. Has to be freed after usage.
 */
char* ipc_read(int _sock) {
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "ipc reading from socket %d\n",_sock);
  if (_sock < 0) {
    syslog(LOG_AUTHPRIV|LOG_ERR, "invalid socket in ipc_read");
    return NULL;
  }
  else {
    int len = 0;
    int rv;
    fd_set set;
    FD_ZERO(&set); 
    FD_SET(_sock, &set); 
    rv = select(_sock + 1, &set, NULL, NULL, NULL);
    if(rv > 0) {
      if(ioctl(_sock, FIONREAD, &len)!=0){
        syslog(LOG_AUTHPRIV|LOG_ERR, "ioctl: %m");
        return NULL;
      }
      if (len > 0) {
        char* buf = calloc(sizeof(char), len+1);
        len = read(_sock, buf, len);
        syslog(LOG_AUTHPRIV|LOG_DEBUG, "ipc read %s\n",buf);
        return buf;
      } else {
        syslog(LOG_AUTHPRIV|LOG_DEBUG, "Client disconnected");
        return NULL;
      }
    }
    else if (rv == -1) {
      syslog(LOG_AUTHPRIV|LOG_ALERT, "error select in ipc_read: %m");
      return NULL;
    } else if (rv == 0) {
      // timeout can not happen with infinite timeout
    }
    return NULL;
  }
}

/** @fn int ipc_write(int _sock, char* msg)
 * @brief writes a message to a socket
 * @param _sock the socket to write to
 * @param msg the msg to be written
 * @return 0 on success; -1 on failure
 */
int ipc_write(int _sock, char* fmt, ...) {
  va_list args, original;
  va_start(original, fmt);
  va_start(args, fmt);
  char* msg = calloc(sizeof(char), vsnprintf(NULL, 0, fmt, args)+1);
  vsprintf(msg, fmt, original);
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "ipc writing to socket %d\n",_sock);
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "ipc write %s\n",msg);
  if (write(_sock, msg, strlen(msg)+1) < 0) {
    syslog(LOG_AUTHPRIV|LOG_ERR, "writing on stream socket: %m");
    free(msg);
    return -1;
  }
  free(msg);
  return 0;
}

/** @fn int ipc_writeWithMode(int _sock, char* msg, int mode)
 * @brief writes a mode and a message to a socket
 * @param _sock the socket to write to
 * @param msg the msg to be written
 * @param mode the mode. Possible values are defined in ipc.h
 * @return 0 on success; -1 on failure
 */
int ipc_writeWithMode(int _sock, int mode, char* fmt, ...) {
  va_list args, original;
  va_start(original, fmt);
  va_start(args, fmt);
  int modeLen = snprintf(NULL,0,"%d",mode);
  char* msg = calloc(sizeof(char), vsnprintf(NULL, 0, fmt, args)+modeLen+1);
  snprintf(msg, modeLen+1, "%d", mode);
  vsprintf(msg+modeLen, fmt, original);
  int res;
  if((res = ipc_write(_sock, msg))!=0) { 
    free(msg);
    return res;
  }
  free(msg);
  return 0;
}

/** @fn int ipc_close(struct connection con)
 * @brief closes an ipc connection
 * @param con, the connection struct
 * @return 0 on success
 */
int ipc_close(struct connection* con) {
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "close ipc\n");
  if(con->sock!=NULL)
    close(*(con->sock));
  if(con->msgsock!=NULL)
    close(*(con->msgsock));
  free(con->server); con->server = NULL;
  free(con->sock); con->sock = NULL;
  free(con->msgsock); con->msgsock = NULL;
  return 0;
}

int ipc_closeAndUnlink(struct connection* con) {
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "Unlinking %s", con->server->sun_path);
  unlink(con->server->sun_path);
  ipc_close(con);
  return 0;
}
