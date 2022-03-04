#include "connection.h"

#ifndef __MINGW32__
#include <unistd.h>
#endif

#include "utils/memory.h"

/** @fn int connection_comparator(const void* v1, const void* v2)
 * @brief compares two connections by their msgsock.
 * @param v1 pointer to the first element
 * @param v2 pointer to the second element
 * @return 1 if v1==v2; 0 else
 */
int connection_comparator(const struct connection* c1,
                          const struct connection* c2) {
#ifdef __MINGW32__
    if (c1->tcp_server == NULL && c2->tcp_server == NULL) {
        return 1;
    }
    if (c1->tcp_server == NULL || c2->tcp_server == NULL) {
        return 0;
    }
    if (c1->tcp_server == c2->tcp_server) {
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
#else // no __MINGW32__
    if (c1->msgsock == NULL && c2->msgsock == NULL) {
        return 1;
    }
    if (c1->msgsock == NULL || c2->msgsock == NULL) {
        return 0;
    }
    if (*(c1->msgsock) == *(c2->msgsock)) {
        return 1;
    }
#endif
  return 0;
}

void _secFreeConnection(struct connection* con) {
  secFree(con->tcp_server);
#ifdef __MINGW32__
  if (con->sock) {
      closesocket(*(con->sock));
  }
#else
  secFree(con->server);
  if (con->msgsock) {
    close(*(con->msgsock));
  }
  secFree(con->msgsock);
#endif
  secFree(con->sock);
  secFree(con);

}
