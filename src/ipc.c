#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <syslog.h>

#define SOCKET_PATH_ENV_VAR "OIDC_SOCKET_PATH"

const char* const dir_const = "/tmp/oidc-XXXXXX";
char* dir = NULL;
char* socket_path = NULL;

int* sock = NULL;
int* msgsock = NULL;
struct sockaddr_un* server = NULL;

int init_socket_path() {
  dir = calloc(sizeof(char), strlen(dir_const));
  strcpy(dir, dir_const);
  mkdtemp(dir);
  pid_t ppid = getppid();
  char* fmt = "%s/token.%d";
  socket_path = calloc(sizeof(char), strlen(dir)+strlen(fmt)+snprintf(NULL, 0, "%d", ppid)+1);
  sprintf(socket_path, fmt, dir, ppid);
  setenv(SOCKET_PATH_ENV_VAR, socket_path, 1);
  return 0;
}

/** @fn int ipc_init()
 * @brief initializes inter process communication
 * @note the initialization will fail, if the socket was not correctly unlinked.
 * To ensure that, call \f ipc_close
 * @return 0 on success, otherwise a negative error code
 */
int ipc_init(int isServer) {
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "initializing ipc\n");
  server = calloc(sizeof(struct sockaddr_un),1);
  sock = calloc(sizeof(int),1);
  msgsock = calloc(sizeof(int),1);
  if(server==NULL || sock==NULL || msgsock==NULL) {
    syslog(LOG_AUTHPRIV|LOG_ALERT, "malloc failed\n");
    exit(EXIT_FAILURE);
  }
  if(isServer)
    init_socket_path();
  else
    socket_path = getenv(SOCKET_PATH_ENV_VAR);
  *sock = socket(AF_UNIX, SOCK_STREAM, 0);
  if (*sock < 0) {
    perror("opening stream socket");
    return *sock;
  }
  server->sun_family = AF_UNIX;
  strcpy(server->sun_path, socket_path); 

  return 0;
}

/** @fn int ipc_bind(void(callback)())
 * @brief binds the server socket, starts listening and accepting a connection
 * @param callback a callback function. It will be called between listen and
 * accepting a connection and can be used to start the communication partner
 * process. Can also be NULL.
 * @return the msgsock or -1 on failure
 */
int ipc_bind(void(callback)()) {
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "binding ipc\n");
  if (bind(*sock, (struct sockaddr *) server, sizeof(struct sockaddr_un))) {
    perror("binding stream socket");
    close(*sock);
    return -1;
  }
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "listen ipc\n");
  listen(*sock, 5);
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "callback ipc\n");
  if (callback)
    callback();
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "accepting ipc\n");
  *msgsock = accept(*sock, 0, 0);
  return *msgsock;
}

/** @fn int ipc_connect()
 * @brief connects to a UNIX Domain socket
 * @return the socket or -1 on failure
 */
int ipc_connect() {
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "connecting ipc\n");
  if (connect(*sock, (struct sockaddr *) server, sizeof(struct sockaddr_un)) < 0) {
    close(*sock);
    perror("connecting stream socket");
    return -1;
  }
  return *sock;
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
int ipc_close() {
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "close ipc\n");
  if(sock!=NULL)
    close(*sock);
  if(msgsock!=NULL)
    close(*msgsock);
  unlink(socket_path);
  free(server); server = NULL;
  free(sock); sock = NULL;
  free(msgsock); msgsock = NULL;
  free(socket_path); socket_path=NULL;
  if(dir)
    rmdir(dir);
  free(dir); dir=NULL;
  return 0;
}
