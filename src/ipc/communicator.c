#include "communicator.h"

#include "../oidc_error.h"
#include "../settings.h"
#include "../utils/printer.h"
#include "ipc.h"

#include <stdlib.h>
#include <syslog.h>

char* communicateWithConnection(char* fmt, va_list args,
                                struct connection* con) {
  if (ipc_connect(*con) < 0) {
    return NULL;
  }
  if (ipc_vwrite(*(con->sock), fmt, args) != OIDC_SUCCESS) {
    return NULL;
  }
  char* response = ipc_read(*(con->sock));
  ipc_close(con);
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

  return ipc_vcommunicate(fmt, args);
}

char* ipc_vcommunicate(char* fmt, va_list args) {
  static struct connection con;
  if (ipc_init(&con, OIDC_SOCK_ENV_NAME, 0) != OIDC_SUCCESS) {
    return NULL;
  }
  return communicateWithConnection(fmt, args, &con);
}

char* ipc_communicateWithPath(char* fmt, ...) {
  va_list args;
  va_start(args, fmt);

  return ipc_vcommunicateWithPath(fmt, args);
}

char* ipc_vcommunicateWithPath(char* fmt, va_list args) {
  static struct connection con;
  if (ipc_initWithPath(&con) != OIDC_SUCCESS) {
    return NULL;
  }
  return communicateWithConnection(fmt, args, &con);
}
