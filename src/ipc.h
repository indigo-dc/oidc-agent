#ifndef IPC_H
#define IPC_H

#include <sys/un.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>

#include <syslog.h>

#include "oidc_array.h"


// #define STATUS_SUCCESS "0"
// #define STATUS_SUCCESS_WITH_REFRESH "1"
// #define STATUS_FAILURE "3"
// #define ENV_VAR_NOT_SET 4
// #define JSON_MALFORMED "5"

#define RESPONSE_STATUS "{\"status\":\"%s\"}"
#define RESPONSE_STATUS_REFRESH "{\"status\":\"%s\", \"refresh_token\":\"%s\"}"
#define RESPONSE_STATUS_ACCESS "{\"status\":\"%s\", \"access_token\":\"%s\"}"
#define RESPONSE_STATUS_PROVIDER "{\"status\":\"%s\", \"provider_list\":\"%s\"}"
#define RESPONSE_ERROR "{\"status\":\"failure\", \"error\":\"%s\"}"


struct connection {
  int* sock;
  int* msgsock;
  struct sockaddr_un* server;
};

char* init_socket_path(const char* prefix, const char* env_var_name) ;
int ipc_init(struct connection* con, const char* prefix, const char* env_var_name, int isServer) ;
int ipc_bind(struct connection* con) ;
int ipc_bindAndListen(struct connection* con) ;
struct connection* ipc_async(struct connection listencon, struct connection** clientcons_addr, size_t* size) ;
int ipc_connect(struct connection con) ;
char* ipc_read(int _sock);
int ipc_write(int _sock, char* msg, ...);
int ipc_close(struct connection* con);
int ipc_closeAndUnlink(struct connection* con);


inline static struct connection* addConnection(struct connection* cons, size_t* size, struct connection client) {
  return arr_addElement(cons, size, sizeof(*cons), &client);    
}

/** @fn int provider_comparator(const void* v1, const void* v2)
 * @brief compares two providers by their name. Can be used for sorting.
 * @param v1 pointer to the first element
 * @param v2 pointer to the second element
 * @return -1 if v1<v2; 1 if v1>v2; 0 if v1=v2
 */
inline static int connection_comparator(const void *v1, const void *v2) {
  const struct connection *c1 = (struct connection *)v1;
  const struct connection *c2 = (struct connection *)v2;
  if(c1->msgsock == NULL && c2->msgsock == NULL)
    return 0;
  if(c1->msgsock == NULL && c2->msgsock !=NULL)
    return -1;
  if(c1->msgsock != NULL && c2->msgsock ==NULL)
    return 1;
  if(*(c1->msgsock) == *(c2->msgsock))
    return 0;
  if(*(c1->msgsock) <= *(c2->msgsock))
    return -1;
  if(*(c1->msgsock) >= *(c2->msgsock))
    return 1;
}

inline static int con_sock_comp(const void *v1, const void *v2) {
  const struct connection *c1 = (struct connection *)v2;
  int *msgsock = (int *)v1;
  if(c1->msgsock)
    syslog(LOG_AUTHPRIV|LOG_DEBUG, "key sock is %d - consock is %d", *msgsock, *(c1->msgsock));
  if(c1->msgsock == NULL )
    return -1;
  if(*(c1->msgsock) == *msgsock)
    return 0;
  if(*(c1->msgsock) <= *msgsock)
    return 1;
  if(*(c1->msgsock) >= *msgsock)
    return -1;
}

/** @fn void sortProvider()
 * @brief sorts providers by their name using \f provider_comparator 
 */
inline static struct connection* sortConnections(struct connection* cons, size_t size) {
  return arr_sort(cons, size, sizeof(struct connection), connection_comparator);
}

inline static struct connection* findConnections(struct connection* cons, size_t size, struct connection key) {
  return arr_find(cons, size, sizeof(struct connection), &key, connection_comparator);
}

inline static struct connection* removeConnection(struct connection* cons, size_t* size, struct connection* key) {
  int msgsock = *(key->msgsock);
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "key sock is %d", msgsock);
  void* pos = arr_find(cons, *size, sizeof(struct connection), &msgsock, con_sock_comp);
  if(NULL==pos) {
    syslog(LOG_AUTHPRIV|LOG_DEBUG, "Did not find key");
    return NULL;
  }
  ipc_close(key);
  memmove(pos, cons + *size - 1, sizeof(struct connection));
  (*size)--;
  cons = realloc(cons, sizeof(struct connection) * (*size));
  return cons;
}


#endif // IPC_H
