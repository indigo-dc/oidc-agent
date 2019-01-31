#include "communicator.h"

#include "defines/settings.h"
#include "ipc.h"
#include "utils/oidc_error.h"
#include "utils/printer.h"

#include <stdlib.h>
#include <syslog.h>

char* communicateWithConnection(char* fmt, va_list args,
                                struct connection* con) {
  if (ipc_connect(*con) < 0) {
    return NULL;
  }
  char* response = ipc_communicateWithSock(*(con->sock), fmt, args);
  ipc_closeConnection(con);
  if (NULL == response) {
    printError("An unexpected error occured. It seems that oidc-agent has "
               "stopped.\n%s\n",
               oidc_serror());
    syslog(LOG_AUTHPRIV | LOG_ERR,
           "An unexpected error occured. It seems that oidc-agent has "
           "stopped.\n%s\n",
           oidc_serror());
    exit(EXIT_FAILURE);
  }
  return response;
}

char* ipc_communicate(char* fmt, ...) {
  va_list args;
  va_start(args, fmt);
  char* ret = ipc_vcommunicate(fmt, args);
  va_end(args);
  return ret;
}

char* ipc_vcommunicate(char* fmt, va_list args) {
  static struct connection con;
  if (ipc_client_init(&con, OIDC_SOCK_ENV_NAME) != OIDC_SUCCESS) {
    return NULL;
  }
  return communicateWithConnection(fmt, args, &con);
}
