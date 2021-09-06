#include "connection.h"

#include "utils/memory.h"


/** @fn int connection_comparator(const void* v1, const void* v2)
 * @brief compares two connections by their msys_server and socket.
 * @param v1 pointer to the first element
 * @param v2 pointer to the second element
 * @return -1 if v1<v2; 1 if v1>v2; 0 if v1=v2
 */
int connection_comparator(const struct connection* c1,
                          const struct connection* c2) {
  if (c1->msys_server == NULL && c2->msys_server == NULL) {
    return 1;
  }
  if (c1->msys_server == NULL || c2->msys_server == NULL) {
    return 0;
  }
  if (c1->msys_server == c2->msys_server) {
    return 1;
  }
  if (c1->sock == NULL && c2->sock == NULL) {
    return 1;
  }
  if (c1->sock == NULL || c2->sock == NULL) {
    return 0;
  }
  if (*(c1->sock) == *(c2->sock)) {
    return 1;
  }
  return 0;
}

void _secFreeConnection(struct connection* con) {
  secFree(con->msys_server);
  con->msys_server = NULL;
  if (con->sock) {
    closesocket(*(con->sock));
  }
  secFree(con->sock);
  con->sock = NULL;
  secFree(con);
}
