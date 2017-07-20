#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <syslog.h>


#define NAME "oidc_socket"

// int sock, msgsock, rval;
int* sock = NULL;
int* msgsock = NULL;
struct sockaddr_un* server = NULL;

int ipc_init() {
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "initializing ipc\n");
  server = calloc(sizeof(struct sockaddr_un),1);
  sock = calloc(sizeof(int),1);
  msgsock = calloc(sizeof(int),1);
  if(server==NULL || sock==NULL || msgsock==NULL) {
    syslog(LOG_AUTHPRIV|LOG_ALERT, "malloc failed\n");
    exit(EXIT_FAILURE);
  }
  *sock = socket(AF_UNIX, SOCK_STREAM, 0);
  if (*sock < 0) {
    perror("opening stream socket");
    return *sock;
  }
  server->sun_family = AF_UNIX;
  strcpy(server->sun_path, NAME); 

  return 0;
}

int ipc_bind(void(callback)()) {
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "binding ipc\n");
  if (bind(*sock, (struct sockaddr *) server, sizeof(struct sockaddr_un))) {
    perror("binding stream socket");
    close(*sock);
    return 1;
  }
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "listen ipc\n");
  listen(*sock, 5);
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "callback ipc\n");
  callback();
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "accepting ipc\n");
  *msgsock = accept(*sock, 0, 0);
  return *msgsock;
}

int ipc_connect() {
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "connecting ipc\n");
  if (connect(*sock, (struct sockaddr *) server, sizeof(struct sockaddr_un)) < 0) {
    close(*sock);
    perror("connecting stream socket");
    return 1;
  }
  return *sock;
}

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

int ipc_write(int _sock, char* msg) {
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "ipc writing to socket %d\n",_sock);
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "ipc write %s\n",msg);
  if (write(_sock, msg, strlen(msg)+1) < 0) {
    perror("writing on stream socket");
    return 1;
  }
  return 0;
}

int ipc_close() {
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "close ipc\n");
  if(sock!=NULL)
    close(*sock);
  if(msgsock!=NULL)
    close(*msgsock);
  unlink(NAME);
  free(server);
  free(sock);
  free(msgsock);
  return 0;
}
