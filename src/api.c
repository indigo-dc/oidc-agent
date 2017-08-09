#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <syslog.h>

#include "api.h"
#include "ipc.h"
#include "config.h"
#include "oidc_string.h"

struct connection* initTokenSocket() {
  // struct connection* con = calloc(sizeof(struct connection), 1);
  // free(init_socket_path(con, "token", "OIDC_API_SOCKET_PATH"));
  // int pid = fork();
  // if(pid==-1) {
  //   syslog(LOG_AUTHPRIV|LOG_ALERT, "fork: %m");
  //   exit(EXIT_FAILURE);
  // } if (pid==0) {
  //   execl("/usr/bin/x-terminal-emulator", "/usr/bin/x-terminal-emulator", (char*) NULL);
  //   exit(EXIT_SUCCESS);
  // }
  // if(ipc_init(con,"token", "OIDC_API_SOCKET_PATH", 1)!=0) {
  //   syslog(LOG_AUTHPRIV|LOG_ALERT, "Could not init socket for token communication.");
  //   closeTokenSocket(con);
  //   exit(EXIT_FAILURE);
  // }
  // ipc_bindAndListen(con);
  // return con; 
}

void closeTokenSocket(struct connection* con) {
  ipc_close(con);
  free(con);
}

int tryAccept(struct connection* con, time_t timeout_s) {
  // return ipc_accept_async(con, timeout_s);
}

char* getProviderList() {
  unsigned int i;
  char* providerList = calloc(sizeof(char), strlen(conf_getProviderName(0))+1);
  sprintf(providerList, "%s", conf_getProviderName(0));
  char* fmt = "%s, %s";
  for(i=1; i<conf_getProviderCount(); i++) {
    providerList = realloc(providerList, strlen(providerList)+strlen(conf_getProviderName(i))+strlen(fmt)+1);
    sprintf(providerList, fmt, providerList, conf_getProviderName(i));
  }
  return providerList;
}

/** @fn void* communicate(void* arg)
 * @brief communicates with other processes that wish to get an access token
 * @param arg, is unused and should be NULL TODO
 */
int communicate(struct connection* con) {
  int msgsock = *(con->msgsock);
  if(msgsock<0) {
    syslog(LOG_AUTHPRIV|LOG_ALERT, "Could not bind socket for token communication");
    closeTokenSocket(con);
    exit(EXIT_FAILURE);
  }
  char* data = ipc_read(msgsock);
  if(data==NULL) {
    syslog(LOG_AUTHPRIV|LOG_ERR, "Did not receive any data");
    return -1;
  } else if(strcmp(data, "provider")==0) { // Provider list
    char* response = getProviderList();
    ipc_write(msgsock, response);
    free(response);
    free(data);
    return 1;
  } else if(strstarts(data, "token/")) { // access_token
    char* provider = data+strlen("token/");
    int providerno = conf_getProviderIndex(provider);
    if(providerno<0) {
      syslog(LOG_AUTHPRIV|LOG_ERR, "Could not find provider %s, therefore no access token found.", provider);
      char* providerList = getProviderList();
      ipc_write(msgsock, "Could not find provider %s. The following providers are configured: %s", provider, providerList); 
      free(providerList);
      free(data);
      return -2;
    } else {
      char* access_token = conf_getAccessToken(providerno);
      ipc_write(msgsock, access_token ? access_token : "Could not get an access token. Provider not correctly configured or you did never provide correct credentials\n.");
      free(data);
      return 2;
    } 
  } else { 
    ipc_write(msgsock, "Malformed query");
    free(data);
    return -3;
  }
}
