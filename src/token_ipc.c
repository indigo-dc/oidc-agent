#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <syslog.h>

#include "ipc.h"
#include "config.h"

#include <unistd.h>

/** @fn void* communicate(void* arg)
 * @brief communicates with other processes that wish to get an access token
 * @param arg, is unused and should be NULL
 * @return this function will never return, on failure it will call exit
 */
void* communicate(void* arg) {
  free(init_socket_path("token", "OIDC_TOKEN_SOCKET_PATH"));
  int pid = fork();
  if(pid==-1) {
    perror("fork");
    exit(EXIT_FAILURE);
  } if (pid==0) {
    system("x-terminal-emulator");
    exit(EXIT_SUCCESS);
  }
  while(1) {
    static struct connection con;
    if(ipc_init(&con,"token", NULL, 1)!=0) {
      syslog(LOG_AUTHPRIV|LOG_ALERT, "Could not init socket for token communication.");
      ipc_close(con);
      exit(EXIT_FAILURE);
    }
    int msgsock = ipc_bind(con, NULL);
    if(msgsock<0) {
      syslog(LOG_AUTHPRIV|LOG_ALERT, "Could not bind socket for token communication");
      ipc_close(con);
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
    ipc_close(con);
    free(provider);
  }
  return NULL;
}
