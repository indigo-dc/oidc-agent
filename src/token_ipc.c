#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <syslog.h>

#include "token_ipc.h"
#include "ipc.h"
#include "config.h"


struct connection* initTokenSocket() {
  free(init_socket_path("token", "OIDC_TOKEN_SOCKET_PATH"));
  int pid = fork();
  if(pid==-1) {
    syslog(LOG_AUTHPRIV|LOG_ALERT, "fork: %m");
    exit(EXIT_FAILURE);
  } if (pid==0) {
    execl("/usr/bin/x-terminal-emulator", "/usr/bin/x-terminal-emulator", (char*) NULL);
    exit(EXIT_SUCCESS);
  }
  struct connection* con = calloc(sizeof(struct connection), 1);
  if(ipc_init(con,"token", "OIDC_TOKEN_SOCKET_PATH", 1)!=0) {
    syslog(LOG_AUTHPRIV|LOG_ALERT, "Could not init socket for token communication.");
    closeTokenSocket(con);
    exit(EXIT_FAILURE);
  }
  ipc_bindAndListen(con);
  return con; 
}

void closeTokenSocket(struct connection* con) {
  ipc_close(*con);
  free(con);
}

int tryAccept(struct connection* con, time_t timeout_s) {
  return ipc_accept_async(con, timeout_s);
}


/** @fn void* communicate(void* arg)
 * @brief communicates with other processes that wish to get an access token
 * @param arg, is unused and should be NULL TODO
 */
void communicate(struct connection* con) {
  int msgsock = *(con->msgsock);
  if(msgsock<0) {
    syslog(LOG_AUTHPRIV|LOG_ALERT, "Could not bind socket for token communication");
    closeTokenSocket(con);
    exit(EXIT_FAILURE);
  }
  char* provider = ipc_read(msgsock);
  if(provider==NULL) {
    syslog(LOG_AUTHPRIV|LOG_ERR, "Did not receive any data");
  }
  int providerno = conf_getProviderIndex(provider);
  if(providerno<0) {
    syslog(LOG_AUTHPRIV|LOG_ERR, "Could not find provider %s, therefore no access token found.", provider);
    ipc_write(msgsock, "Could not find this provider."); //TODO send providername and list of possible providers
  } else {
    char* access_token = conf_getAccessToken(providerno);
    ipc_write(msgsock, access_token ? access_token : "Could not get an access token. Provider not correctly configured or you did never provide correct credentials\n.");
  }
  free(provider);
}
