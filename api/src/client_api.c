#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <syslog.h>

#include "client_api.h"
#include "../../src/ipc.h"

#define LOG_LEVEL LOG_DEBUG

char* getAccessToken(const char* providername) {
  openlog("oidc-api", LOG_CONS|LOG_PID, LOG_AUTHPRIV);
  setlogmask(LOG_UPTO(LOG_LEVEL));
  if(providername==NULL) {
    syslog(LOG_AUTHPRIV|LOG_ERR, "No provider specified");
    return NULL;
  }
  static struct connection con;
  if(ipc_init(&con, "api", "OIDC_API_SOCKET_PATH", 0)==DAEMON_NOT_RUNNING) {
    syslog(LOG_AUTHPRIV|LOG_ERR, "Could not init socket, because env var not set. Daemon not running.");
    return NULL;
  }
  int sock = ipc_connect(con);
  if(sock<0) {
    syslog(LOG_AUTHPRIV|LOG_ALERT, "Could not connect to socket");
    ipc_close(&con);
    return NULL;
  }
  ipc_write(sock, "token/%s", providername);
  char* access_token = ipc_read(sock);
  ipc_close(&con);
  closelog();
  return access_token;
}

char* getProviderList() {
  openlog("oidc-api", LOG_CONS|LOG_PID, LOG_AUTHPRIV);
  setlogmask(LOG_UPTO(LOG_LEVEL));
  static struct connection con;
  if(ipc_init(&con, "api", "OIDC_API_SOCKET_PATH", 0)==DAEMON_NOT_RUNNING) {
    syslog(LOG_AUTHPRIV|LOG_ERR, "Could not init socket, because env var not set. Daemon not running.");
    return NULL;
  }
  int sock = ipc_connect(con);
  if(sock<0) {
    syslog(LOG_AUTHPRIV|LOG_ALERT, "Could not connect to socket");
    ipc_close(&con);
    return NULL;
  }
  ipc_write(sock, "provider");
  char* providerList = ipc_read(sock);
  ipc_close(&con);
  closelog();
  return providerList;

}

