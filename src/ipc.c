#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <syslog.h>

#include "ipc.h"

const char* const dir_const = "/tmp/oidc-XXXXXX";
// char* dir = NULL;
const char* const dir = "/.oidc";


char* init_socket_path(const char* prefix, const char* env_var_name) {
  // if (dir==NULL) {
  // dir = calloc(sizeof(char), strlen(dir_const)+1);
  // strcpy(dir, dir_const);
  // if (mkdtemp(dir)==NULL)
  //   syslog(LOG_AUTHPRIV|LOG_ALERT, "%m");
  // }
  pid_t ppid = getppid();
  char* fmt = "%s/%s.%d";
  char* socket_path = calloc(sizeof(char), strlen(dir)+strlen(fmt)+strlen(prefix)+snprintf(NULL, 0, "%d", ppid)+1);
  sprintf(socket_path, fmt, dir, prefix, ppid);
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "Setting env var '%s' to '%s'", env_var_name, socket_path);
  if(env_var_name)
    setenv(env_var_name, socket_path, 1);
  return socket_path;
}

/** @fn int ipc_init()
 * @brief initializes inter process communication
 * @note the initialization will fail, if the socket was not correctly unlinked.
 * To ensure that, call \f ipc_close
 * @return 0 on success, otherwise a negative error code
 */
int ipc_init(struct connection* con, const char* prefix, const char* env_var_name, int isServer) {
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "initializing ipc\n");
  con->server = calloc(sizeof(struct sockaddr_un),1);
  con->sock = calloc(sizeof(int),1);
  con->msgsock = calloc(sizeof(int),1);
  if(con->server==NULL || con->sock==NULL || con->msgsock==NULL) {
    syslog(LOG_AUTHPRIV|LOG_ALERT, "malloc failed\n");
    exit(EXIT_FAILURE);
  }

  *(con->sock) = socket(AF_UNIX, SOCK_STREAM, 0);
  if (*(con->sock) < 0) {
    perror("opening stream socket");
    return *(con->sock);
  }
  con->server->sun_family = AF_UNIX;
  if(isServer) {
    char* path = init_socket_path(prefix, env_var_name);
    strcpy(con->server->sun_path, path);
    free(path);
  }
  else {
    char* path = getenv(env_var_name);
    if(path==NULL)
      return DAEMON_NOT_RUNNING;
    strcpy(con->server->sun_path, path); 
  }

  return 0;
}

/** @fn int ipc_bind(void(callback)())
 * @brief binds the server socket, starts listening and accepting a connection
 * @param callback a callback function. It will be called between listen and
 * accepting a connection and can be used to start the communication partner
 * process. Can also be NULL.
 * @return the msgsock or -1 on failure
 */
int ipc_bind(struct connection con, void(callback)()) {
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "binding ipc\n");
  unlink(con.server->sun_path);
  if (bind(*(con.sock), (struct sockaddr *) con.server, sizeof(struct sockaddr_un))) {
    syslog(LOG_AUTHPRIV|LOG_ALERT, "binding stream socket: %m");
    close(*(con.sock));
    return -1;
  }
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "listen ipc\n");
  listen(*(con.sock), 5);
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "callback ipc\n");
  if (callback)
    callback();
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "accepting ipc\n");
  *(con.msgsock) = accept(*(con.sock), 0, 0);
  return *(con.msgsock);
}

/** @fn int ipc_connect()
 * @brief connects to a UNIX Domain socket
 * @return the socket or -1 on failure
 */
int ipc_connect(struct connection con) {
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "connecting ipc\n");
  if (connect(*(con.sock), (struct sockaddr *) con.server, sizeof(struct sockaddr_un)) < 0) {
    close(*(con.sock));
    perror("connecting stream socket");
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
  if (_sock == -1)
    perror("accept");
  else {
    int len = 0;
    while(len<=0){
      if(ioctl(_sock, FIONREAD, &len)!=0){
        perror("ioctl");
      }
      if (len > 0) {
        char* buf = calloc(sizeof(char), len+1);
        len = read(_sock, buf, len);
        syslog(LOG_AUTHPRIV|LOG_DEBUG, "ipc read %s\n",buf);
        return buf;
      }
      usleep(1000);
    }
  }
  return NULL;
}

/** @fn int ipc_write(int _sock, char* msg)
 * @brief writes a message to a socket
 * @param _sock the socket to write to
 * @param msg the msg to be written
 * @return 0 on success; -1 on failure
 */
int ipc_write(int _sock, char* msg) {
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "ipc writing to socket %d\n",_sock);
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "ipc write %s\n",msg);
  if (write(_sock, msg, strlen(msg)+1) < 0) {
    perror("writing on stream socket");
    return -1;
  }
  return 0;
}

int ipc_writeWithMode(int _sock, char* msg, int mode) {
  char* toSend = calloc(sizeof(char), strlen(msg)+1+1);
  sprintf(toSend, "%d%s", mode, msg);
  int res;
  if((res = ipc_write(_sock, toSend))!=0) { 
    free(toSend);
    return res;
  }
  free(toSend);
  return 0;
}

/** @fn int ipc_close()
 * @brief closes an ipc connection
 * @return 0 on success
 */
int ipc_close(struct connection con) {
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "close ipc\n");
  if(con.sock!=NULL)
    close(*(con.sock));
  if(con.msgsock!=NULL)
    close(*(con.msgsock));
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "Unlinking %s", con.server->sun_path);
  unlink(con.server->sun_path);
  free(con.server); con.server = NULL;
  free(con.sock); con.sock = NULL;
  free(con.msgsock); con.msgsock = NULL;
  // if(dir)
  //   rmdir(dir); // removes directory only if it is empty
  // free(dir); dir=NULL;
  return 0;
}
