#include "tcp_serveripc.h"

#include <arpa/inet.h>
#include <sys/fcntl.h>
#include <unistd.h>

#include "ipc/ipc.h"
#include "utils/logger.h"

/**
 * @brief initializes a server tcp socket
 * @param con, a pointer to the connection struct. The relevant fields will be
 * initialized.
 */
oidc_error_t ipc_tcp_server_init(struct connection* con, unsigned short port) {
  logger(DEBUG, "initializing server ipc");
  if (initConnectionWithoutPath(con, 1, 1) != OIDC_SUCCESS) {
    return oidc_errno;
  }

  con->tcp_server->sin_port        = htons(port);
  con->tcp_server->sin_addr.s_addr = INADDR_ANY;
  // con->tcp_server->sin_addr.s_addr = inet_addr("127.0.0.1");

  return OIDC_SUCCESS;
}

/**
 * @brief binds the server socket and starts listening
 * @param con, a pointer to the connection struct
 * @return @c 0 on success or an errorcode on failure
 */
int ipc_tcp_bindAndListen(struct connection* con) {
  logger(DEBUG, "binding tcp ipc\n");
  if (bind(*(con->sock), (struct sockaddr*)con->tcp_server,
           sizeof(struct sockaddr_in))) {
    logger(ALERT, "binding stream socket: %m");
    close(*(con->sock));
    oidc_errno = OIDC_EBIND;
    return OIDC_EBIND;
  }

  int flags;
  if (-1 == (flags = fcntl(*(con->sock), F_GETFL, 0)))
    flags = 0;
  fcntl(*(con->sock), F_SETFL, flags | O_NONBLOCK);

  logger(DEBUG, "listen ipc\n");
  return listen(*(con->sock), 5);
}
