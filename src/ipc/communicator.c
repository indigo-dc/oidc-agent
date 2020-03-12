#include "communicator.h"

#include "defines/settings.h"
#include "ipc.h"
#include "utils/logger.h"
#include "utils/oidc_error.h"
#include "utils/printer.h"

#include <stdlib.h>

char* communicateWithConnection(const char* fmt, va_list args,
                                struct connection* con) {
  if (ipc_connect(*con) < 0) {
    return NULL;
  }
  char* response = ipc_vcommunicateWithSock(*(con->sock), fmt, args);
  ipc_closeConnection(con);
  if (NULL == response) {
    printError("An unexpected error occurred. It seems that oidc-agent has "
               "stopped.\n%s\n",
               oidc_serror());
    logger(ERROR,
           "An unexpected error occurred. It seems that oidc-agent has "
           "stopped.\n%s\n",
           oidc_serror());
    exit(EXIT_FAILURE);
  }
  return response;
}

char* ipc_communicate(const char* fmt, ...) {
  va_list args;
  va_start(args, fmt);
  char* ret = ipc_vcommunicate(fmt, args);
  va_end(args);
  return ret;
}

char* ipc_vcommunicate(const char* fmt, va_list args) {
  static struct connection con;
  if (ipc_client_init(&con, OIDC_SOCK_ENV_NAME) != OIDC_SUCCESS) {
    return NULL;
  }
  return communicateWithConnection(fmt, args, &con);
}
