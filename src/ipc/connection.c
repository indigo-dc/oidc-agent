#include "connection.h"

#include "../utils/cleaner.h"

#include <sys/un.h>

/** @fn int connection_comparator(const void* v1, const void* v2)
 * @brief compares two connections by their msgsock. Can be used for sorting.
 * @param v1 pointer to the first element
 * @param v2 pointer to the second element
 * @return -1 if v1<v2; 1 if v1>v2; 0 if v1=v2
 */
int connection_comparator(const struct connection* c1, const struct connection* c2) {
  if(c1->msgsock == NULL && c2->msgsock == NULL) {
    return 1;
  }
  if(c1->msgsock == NULL || c2->msgsock ==NULL) {
    return 0;
  }
  if(*(c1->msgsock) == *(c2->msgsock)) {
    return 1;
  }
  return 0;
}

void clearFreeConnection(struct connection* con) {
  clearFree(con->server, sizeof(*(con->server))); con->server = NULL;
  clearFree(con->sock, sizeof(*(con->sock))); con->sock = NULL;
  clearFree(con->msgsock, sizeof(*(con->msgsock))); con->msgsock = NULL;
  clearFree(con, sizeof(con));
}
