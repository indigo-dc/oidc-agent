#include "connection.h"

#include <unistd.h>

#include "utils/memory.h"

/** @fn int connection_comparator(const void* v1, const void* v2)
 * @brief compares two connections by their msgsock.
 * @param v1 pointer to the first element
 * @param v2 pointer to the second element
 * @return -1 if v1<v2; 1 if v1>v2; 0 if v1=v2
 */
int connection_comparator(const struct connection* c1,
                          const struct connection* c2) {
  if (c1->msgsock == NULL && c2->msgsock == NULL) {
    return 1;
  }
  if (c1->msgsock == NULL || c2->msgsock == NULL) {
    return 0;
  }
  if (*(c1->msgsock) == *(c2->msgsock)) {
    return 1;
  }
  return 0;
}

void _secFreeConnection(struct connection* con) {
  secFree(con->server);
  con->server = NULL;
  secFree(con->tcp_server);
  con->tcp_server = NULL;
  secFree(con->sock);
  con->sock = NULL;
  if (con->msgsock) {
    close(*(con->msgsock));
  }
  secFree(con->msgsock);
  con->msgsock = NULL;
  secFree(con);
}
