#include "connection.h"

#include "../oidc_array.h"
#include "../oidc_error.h"
#include "ipc.h"

#include <string.h>
#include <syslog.h>

/** @fn struct connection* addConnection(struct connection* cons, size_t* size,
 * struct connection client)
 * @brief adds a connection to an array of connections
 * @param cons a pointer to the start of an array of connections
 * @param size a pointer to the number of elements in the array
 * @param client the connection to be added
 * @return a pointer to the new array
 */
struct connection* addConnection(struct connection* cons, size_t* size, struct connection client) {
  void* tmp = realloc(cons, sizeof(struct connection) * (*size + 1));
  if(tmp==NULL) {
    syslog(LOG_AUTHPRIV|LOG_EMERG, "%s (%s:%d) realloc() failed: %m\n", __func__, __FILE__, __LINE__);
    oidc_errno = OIDC_EALLOC;
    return NULL;
  }
  cons = tmp;
  memcpy(cons + *size, &client, sizeof(struct connection));
  (*size)++;
  // For some reason using the following function insted of the above same
  // statements doesn't work.
  // p= arr_addElement(p, size, sizeof(struct oidc_account), &account);    
  return cons;    
}

/** @fn int connection_comparator(const void* v1, const void* v2)
 * @brief compares two connections by their msgsock. Can be used for sorting.
 * @param v1 pointer to the first element
 * @param v2 pointer to the second element
 * @return -1 if v1<v2; 1 if v1>v2; 0 if v1=v2
 */
int connection_comparator(const void *v1, const void *v2) {
  const struct connection *c1 = (struct connection *)v1;
  const struct connection *c2 = (struct connection *)v2;
  if(c1->msgsock == NULL && c2->msgsock == NULL) {
    return 0;
  }
  if(c1->msgsock == NULL && c2->msgsock !=NULL) {
    return -1;
  }
  if(c1->msgsock != NULL && c2->msgsock ==NULL) {
    return 1;
  }
  if(*(c1->msgsock) == *(c2->msgsock)) {
    return 0;
  }
  if(*(c1->msgsock) <= *(c2->msgsock)) {
    return -1;
  }
  if(*(c1->msgsock) >= *(c2->msgsock)) {
    return 1;
  }
  return -2;
}

/** @fn struct connection* sortConnections()
 * @brief sorts connections  using \f connection_comparator
 * @param cons the array to be sorted
 * @param size the nubmer of elements in cons
 * @return the sorted array
 */
struct connection* sortConnections(struct connection* cons, size_t size) {
  return arr_sort(cons, size, sizeof(struct connection), connection_comparator);
}

/** @fn struct connection* findConnection(struct connection* cons, size_t size,
 * struct connection key)
 * @brief finds a connection socket.
 * @param cons the connection array that should be searched
 * @param size the number of connections in cons
 * @param key the connection to be find. only the msgsock will be compared.
 * @return a pointer to the found connection. If no connection could be found
 * NULL is returned.
 */
struct connection* findConnection(struct connection* cons, size_t size, struct connection key) {
  return arr_find(cons, size, sizeof(struct connection), &key, connection_comparator);
}

/** @fn struct connection* removeConnection(struct connection* cons, size_t*
 * size, struct connection* key)
 * @brief removes a connection from an array of connections, and closes the
 * connection
 * @param cons a pointer to the start of an array of connections
 * @param size a pointer to the number of elements in the array
 * @param key the connection to be removed
 * @return a pointer to the new array
 */
struct connection* removeConnection(struct connection* cons, size_t* size, struct connection* key) {
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "key sock is %d", *(key->msgsock));
  struct connection* pos = findConnection(cons, *size, *key);
  if(NULL==pos) {
    syslog(LOG_AUTHPRIV|LOG_DEBUG, "Did not find key");
    return cons;
  }
  ipc_close(key);
  memmove(pos, cons + *size - 1, sizeof(struct connection));
  (*size)--;
  void* tmp = realloc(cons, sizeof(struct connection) * (*size));
  if(tmp==NULL && *size>0) {
    syslog(LOG_AUTHPRIV|LOG_EMERG, "%s (%s:%d) realloc() failed: %m\n", __func__, __FILE__, __LINE__);
    oidc_errno = OIDC_EALLOC;
    return NULL;
  }
  cons = tmp;
  return cons;
}
