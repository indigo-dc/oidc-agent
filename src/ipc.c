#define _XOPEN_SOURCE 700

#include "ipc.h"
#include "oidc_array.h"
#include "oidc_utilities.h"

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <unistd.h>
#include <syslog.h>

#include <sys/un.h>
#include <sys/ioctl.h>
#include <sys/fcntl.h>
#include <sys/socket.h>
#include <sys/select.h>

#define SOCKET_DIR "/tmp/oidc-XXXXXX"

char* dir = NULL;

/** @fn char* init_socket_path(const char* env_var_name)
 * @brief generates the socket path and prints commands for setting env vars
 * @param env_var_name the name of the environment variable which will be set.
 * If NULL non will be set.
 * @return a pointer to the socket_path. Has to be freed after usage.
 */
char* init_socket_path(const char* env_var_name) {
  if(NULL==dir) {
    dir = calloc(sizeof(char), strlen(SOCKET_DIR)+1);
    strcpy(dir, SOCKET_DIR);
    if(mkdtemp(dir)==NULL) {
      syslog(LOG_AUTHPRIV|LOG_ALERT, "%m");
      oidc_errno = OIDC_EMKTMP;
      return NULL;
    }
  }
  pid_t ppid = getppid();
  char* prefix = "oidc-agent";
  char* fmt = "%s/%s.%d";
  char* socket_path = oidc_sprintf(fmt, dir, prefix, ppid);
  if(env_var_name) {
    // printf("You have to set env var '%s' to '%s'. Please use the following statement:\n", env_var_name, socket_path);
    printf("%s=%s; export %s;\n", env_var_name, socket_path, env_var_name);
  }
  return socket_path;
}

/** @fn int ipc_init(struct connection* con, const char* env_var_name, int isServer)
 * @brief initializes unix domain socket
 * @param con, a pointer to the connection struct. The relevant fields will be
 * initialized.
 * @param env_var_name, the socket_path environment variable name @see
 * init_socket_path
 * @param isServer, specifies if the function is called from a server or client
 * @return 0 on success, otherwise a negative error code
 */
oidc_error_t ipc_init(struct connection* con, const char* env_var_name, int isServer) {
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "initializing ipc\n");
  con->server = calloc(sizeof(struct sockaddr_un),1);
  con->sock = calloc(sizeof(int),1);
  if(isServer) {
    con->msgsock = calloc(sizeof(int),1); // msgsock is not needed for a client; furthermore if the client calls ipc_close it would close stdin
  }
  if(con->server==NULL || con->sock==NULL || (con->msgsock==NULL && isServer)) {
    syslog(LOG_AUTHPRIV|LOG_ALERT, "alloc failed\n");
    exit(EXIT_FAILURE);
  }

  *(con->sock) = socket(AF_UNIX, SOCK_SEQPACKET, 0);
  if(*(con->sock) < 0) {
    syslog(LOG_AUTHPRIV|LOG_ERR, "opening stream socket: %m");
    oidc_errno = OIDC_ECRSOCK;
    return oidc_errno;
  }
  con->server->sun_family = AF_UNIX;

  if(isServer) {
    char* path = init_socket_path(env_var_name);
    if(path == NULL) {
      return oidc_errno;
    }
    strcpy(con->server->sun_path, path);
    clearFreeString(path);
    server_socket_path = con->server->sun_path; 
  } else {
    char* path = getenv(env_var_name);
    if(path==NULL) {
      printError("Could not get the socket path from env var '%s'. Have you started oidc-agent and set the env var?\n", env_var_name);
      syslog(LOG_AUTHPRIV|LOG_WARNING, "Could not get the socket path from env var '%s'", env_var_name);
      oidc_errno = OIDC_EENVVAR;
      return OIDC_EENVVAR;
    } else {
      strcpy(con->server->sun_path, path); 
    }

  }
  return OIDC_SUCCESS;
}

/** @fn int ipc_initWithPath(struct connection* con)
 * @brief initializes unix domain socket with the current server_socket_path
 * @param con, a pointer to the connection struct. The relevant fields will be
 * initialized.
 * @return 0 on success, otherwise a negative error code
 */
oidc_error_t ipc_initWithPath(struct connection* con) {
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "initializing ipc with path %s\n", server_socket_path);
  con->server = calloc(sizeof(struct sockaddr_un),1);
  con->sock = calloc(sizeof(int),1);
  if(con->server==NULL || con->sock==NULL ) {
    syslog(LOG_AUTHPRIV|LOG_ALERT, "alloc failed\n");
    exit(EXIT_FAILURE);
  }

  *(con->sock) = socket(AF_UNIX, SOCK_SEQPACKET, 0);
  if(*(con->sock) < 0) {
    syslog(LOG_AUTHPRIV|LOG_ERR, "opening stream socket: %m");
    oidc_errno = OIDC_ECRSOCK;
    return oidc_errno;
  }
  con->server->sun_family = AF_UNIX;


  if(server_socket_path==NULL) {
    oidc_setArgNullFuncError(__func__);
    return oidc_errno;
  } else {
    strcpy(con->server->sun_path, server_socket_path); 
  }

  return OIDC_SUCCESS;
}

/** @fn int ipc_bind(struct connection con)
 * @brief binds the server socket,  listen and starts accepting a connection
 * @deprecated server should use async ipc. Use \f ipc_bindAndListen instead.
 * @param con, the connection struct
 * @return the msgsock or OIDC_EBIND on failure
 */
int ipc_bind(struct connection* con) {
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "binding ipc\n");
  unlink(con->server->sun_path);
  if(bind(*(con->sock), (struct sockaddr *) con->server, sizeof(struct sockaddr_un))) {
    syslog(LOG_AUTHPRIV|LOG_ALERT, "binding stream socket: %m");
    close(*(con->sock));
    oidc_errno = OIDC_EBIND;
    return OIDC_EBIND;
  }
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "listen ipc\n");
  listen(*(con->sock), 5);
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "accepting ipc\n");
  *(con->msgsock) = accept(*(con->sock), 0, 0);
  return *(con->msgsock);
}

/** @fn int ipc_bindAndListen(struct connection con)
 * @brief binds the server socket and listen 
 * @param con, the connection struct
 * @return 0 on success or errorcode on failure
 */
int ipc_bindAndListen(struct connection* con) {
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "binding ipc\n");
  unlink(con->server->sun_path);
  if(bind(*(con->sock), (struct sockaddr *) con->server, sizeof(struct sockaddr_un))) {
    syslog(LOG_AUTHPRIV|LOG_ALERT, "binding stream socket: %m");
    close(*(con->sock));
    oidc_errno = OIDC_EBIND;
    return OIDC_EBIND;
  }
  int flags;
  if(-1 == (flags = fcntl(*(con->sock), F_GETFL, 0)))
    flags = 0;
  fcntl(*(con->sock), F_SETFL, flags | O_NONBLOCK);

  syslog(LOG_AUTHPRIV|LOG_DEBUG, "listen ipc\n");
  return listen(*(con->sock), 5);
}

/** @fn struct connection* ipc_async(struct connection listencon, struct
 * connection** clientcons_addr, size_t* size)
 * @brief handles asynchronous communication
 *
 * listens for incoming connections on the listencon and for incoming messages
 * on multiple client sockets. If a new client connects it is added to the list
 * of current client connections.  If on any client socket is a message
 * available for reading, a pointer to this connection is returned.
 * @param listencon the connection struct for the socket accepting new client
 * connections. The list is updated if a new client connects.
 * @param clientcons_addr a pointer to an array of client connections
 * @param size a pointer to the number of client connections. The number is
 * updated if a new client connects.
 * @return A pointer to a client connection. On this connection is either a
 * message avaible for reading or the client disconnected.
 */
struct connection* ipc_async(struct connection listencon, struct connection** clientcons_addr, size_t* size) {
  while(1){
    int maxSock = -1;
    fd_set readSockSet;
    FD_ZERO(&readSockSet);
    FD_SET(*(listencon.sock), &readSockSet);  
    if(*(listencon.sock) > maxSock) {
      maxSock = *(listencon.sock);
    }

    unsigned int i;
    for (i=0; i<*size; i++) {
      FD_SET(*((*clientcons_addr+i)->msgsock), &readSockSet);
      if(*((*clientcons_addr+i)->msgsock) > maxSock) {
        maxSock = *((*clientcons_addr+i)->msgsock);
      }
    }

    syslog(LOG_AUTHPRIV|LOG_DEBUG, "Selecting maxSock is %d", maxSock);
    int ret = select(maxSock+1, &readSockSet, NULL, NULL, NULL);
    if(ret >= 0) {
      if(FD_ISSET(*(listencon.sock), &readSockSet)) {
        syslog(LOG_AUTHPRIV|LOG_DEBUG, "New incoming client");
        struct connection newClient = {0, 0, 0};
        newClient.msgsock = calloc(sizeof(int), 1);
        *(newClient.msgsock) = accept(*(listencon.sock), 0, 0);
        if(*(newClient.msgsock) >= 0) {
          syslog(LOG_AUTHPRIV|LOG_DEBUG, "accepted new client sock: %d", *(newClient.msgsock));

          *clientcons_addr = addConnection(*clientcons_addr, size, newClient);
          syslog(LOG_AUTHPRIV|LOG_DEBUG, "updated client list");
        }
        else {
          syslog(LOG_AUTHPRIV|LOG_ERR, "%m");
        }
      }

      int j;
      for (j=*size-1; j>=0; j--) {
        syslog(LOG_AUTHPRIV|LOG_DEBUG, "Checking client %d", *((*clientcons_addr+j)->msgsock));
        if(FD_ISSET(*((*clientcons_addr+j)->msgsock), &readSockSet)) {
          syslog(LOG_AUTHPRIV|LOG_DEBUG, "New message for read av");
          return *clientcons_addr+j;           
        }
      }
    }
    else {
      syslog(LOG_AUTHPRIV|LOG_ERR, "%m");
    }
  }
  return NULL;
}

/** @fn int ipc_connect(struct connection con)
 * @brief connects to a UNIX Domain socket
 * @param con, the connection struct
 * @return the socket or OIDC_ECONSOCK on failure
 */
int ipc_connect(struct connection con) {
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "connecting ipc %s\n", con.server->sun_path);
  if(connect(*(con.sock), (struct sockaddr *) con.server, sizeof(struct sockaddr_un)) < 0) {
    close(*(con.sock));
    syslog(LOG_AUTHPRIV|LOG_ERR, "connecting stream socket: %m");
    oidc_errno = OIDC_ECONSOCK;
    return OIDC_ECONSOCK;
  }
  return *(con.sock);
}

/** @fn char* ipc_read(int _sock)
 * @brief reads from a socket
 * @param _sock the socket to read from
 * @return a pointer to the readed content. Has to be freed after usage. If NULL
 * is returned, it's most likely that the other party disconnected.
 */
char* ipc_read(int _sock) {
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "ipc reading from socket %d\n",_sock);
  if(_sock < 0) {
    syslog(LOG_AUTHPRIV|LOG_ERR, "invalid socket in ipc_read");
    oidc_errno = OIDC_ESOCKINV;
    return NULL;
  } else {
    int len = 0;
    int rv;
    fd_set set;
    FD_ZERO(&set); 
    FD_SET(_sock, &set); 
    rv = select(_sock + 1, &set, NULL, NULL, NULL);
    if(rv > 0) {
      if(ioctl(_sock, FIONREAD, &len)!=0) {
        syslog(LOG_AUTHPRIV|LOG_ERR, "ioctl: %m");
        oidc_errno = OIDC_EIOCTL;
        return NULL;
      }
      if(len > 0) {
        char* buf = calloc(sizeof(char), len+1);
        len = read(_sock, buf, len);
        syslog(LOG_AUTHPRIV|LOG_DEBUG, "ipc read %s\n",buf);
        return buf;
      } else {
        syslog(LOG_AUTHPRIV|LOG_DEBUG, "Client disconnected");
        oidc_errno = OIDC_EIPCDIS;
        return NULL;
      }
    } else if (rv == -1) {
      syslog(LOG_AUTHPRIV|LOG_ALERT, "error select in ipc_read: %m");
      oidc_errno = OIDC_ESELECT;
      return NULL;
    } else if (rv == 0) {
      // timeout can not happen with infinite timeout
    }
    oidc_errno = OIDC_EERROR;
    return NULL;
  }
}

/** @fn int ipc_write(int _sock, char* msg)
 * @brief writes a message to a socket
 * @param _sock the socket to write to
 * @param msg the msg to be written
 * @return 0 on success; -1 on failure
 */
oidc_error_t ipc_write(int _sock, char* fmt, ...) {
  va_list args, original;
  va_start(original, fmt);
  va_start(args, fmt);
  char* msg = calloc(sizeof(char), vsnprintf(NULL, 0, fmt, args)+1);
  vsprintf(msg, fmt, original);
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "ipc writing to socket %d\n",_sock);
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "ipc write %s\n",msg);
  if(write(_sock, msg, strlen(msg)) < 0) {
    syslog(LOG_AUTHPRIV|LOG_ALERT, "writing on stream socket: %m");
    clearFreeString(msg);
    oidc_errno = OIDC_EWRITE;
    return oidc_errno;
  }
  clearFreeString(msg);
  return OIDC_SUCCESS;
}

oidc_error_t ipc_vwrite(int _sock, char* fmt, va_list args) {
  va_list original;
  va_copy(original, args);
  char* msg = calloc(sizeof(char), vsnprintf(NULL, 0, fmt, args)+1);
  vsprintf(msg, fmt, original);
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "ipc writing to socket %d\n",_sock);
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "ipc write %s\n",msg);
  if(write(_sock, msg, strlen(msg)) < 0) {
    syslog(LOG_AUTHPRIV|LOG_ALERT, "writing on stream socket: %m");
    clearFreeString(msg);
    oidc_errno = OIDC_EWRITE;
    return oidc_errno;
  }
  clearFreeString(msg);
  return OIDC_SUCCESS;
}

oidc_error_t ipc_writeOidcErrno(int sock) {
  return ipc_write(sock, RESPONSE_ERROR, oidc_serror());
}

/** @fn int ipc_close(struct connection con)
 * @brief closes an ipc connection
 * @param con, the connection struct
 * @return 0 on success
 */
oidc_error_t ipc_close(struct connection* con) {
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "close ipc\n");
  if(con->sock!=NULL) {
    close(*(con->sock));
  }
  if(con->msgsock!=NULL) {
    close(*(con->msgsock));
  }
  clearFree(con->server, sizeof(*(con->server))); con->server = NULL;
  clearFree(con->sock, sizeof(*(con->sock))); con->sock = NULL;
  clearFree(con->msgsock, sizeof(*(con->msgsock))); con->msgsock = NULL;
  return OIDC_SUCCESS;
}

/** @fn int ipc_closeAndUnlink(struct connection con)
 * @brief closes an ipc connectiona and removes the socket
 * @param con, the connection struct
 * @return 0 on success
 */
oidc_error_t ipc_closeAndUnlink(struct connection* con) {
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "Unlinking %s", con->server->sun_path);
  unlink(con->server->sun_path);
  ipc_close(con);
  return OIDC_SUCCESS;
}



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
