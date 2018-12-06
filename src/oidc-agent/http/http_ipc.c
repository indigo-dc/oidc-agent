#define _GNU_SOURCE
#include "http_ipc.h"
#include "ipc/cryptCommunicator.h"
#include "ipc/cryptIpc.h"
#include "ipc/pipe.h"
#include "utils/oidc_error.h"

#include <fcntl.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <syslog.h>
#include <unistd.h>

char* _handleParent(struct ipcPipe pipes) {
  char* e = server_ipc_read(pipes.rx, pipes.tx);
  closeIpcPipes(pipes);
  if (e == NULL) {
    return NULL;
  }
  char*    end   = NULL;
  long int error = strtol(e, &end, 10);
  if (error) {
    secFree(e);
    oidc_errno = error;
    syslog(LOG_AUTHPRIV | LOG_ERR, "Error from http request: %s",
           oidc_serror());
    return NULL;
  }
  if (*end != '\0') {
    syslog(LOG_AUTHPRIV | LOG_DEBUG, "Received response: %s", e);
    return e;
  }
  secFree(e);
  syslog(LOG_AUTHPRIV | LOG_ERR, "Internal error: Http sent 0");
  return NULL;
}

void handleChild(char* res, struct ipcPipe pipes) {
  if (res != NULL) {
    struct ipc_keySet* ipc_keys =
        client_ipc_writeToSock(pipes.rx, pipes.tx, res);
    if (ipc_keys == NULL) {
      syslog(LOG_AUTHPRIV | LOG_ERR, "%s", oidc_serror());
    }
    secFreeIpcKeySet(ipc_keys);
    closeIpcPipes(pipes);
    exit(EXIT_SUCCESS);
  }
  struct ipc_keySet* ipc_keys = client_ipc_writeToSock(pipes.rx, pipes.tx, res);
  if (ipc_keys == NULL) {
    syslog(LOG_AUTHPRIV | LOG_ERR, "%s", oidc_serror());
  }
  secFreeIpcKeySet(ipc_keys);
  closeIpcPipes(pipes);
  exit(EXIT_FAILURE);
}

/** @fn char* httpsGET(const char* url, const char* cert_path)
 * @brief forks and does a https GET request
 * @param url the request url
 * @param cert_path the path to the SSL certs
 * @return a pointer to the response. Has to be freed after usage. If the Https
 * call failed, NULL is returned.
 */
char* httpsGET(const char* url, struct curl_slist* headers,
               const char* cert_path) {
  struct pipeSet pipes = ipc_pipe_init();
  if (pipes.pipe1.rx == -1) {
    return NULL;
  }
  pid_t pid = fork();
  if (pid == -1) {
    syslog(LOG_AUTHPRIV | LOG_ALERT, "fork %m");
    oidc_setErrnoError();
    return NULL;
  }
  if (pid == 0) {  // child
    struct ipcPipe childPipes = toClientPipes(pipes);
    char*          res        = _httpsGET(url, headers, cert_path);
    handleChild(res, childPipes);
    return NULL;
  } else {  // parent
    signal(SIGCHLD, SIG_IGN);
    struct ipcPipe parentPipes = toServerPipes(pipes);
    return _handleParent(parentPipes);
  }
}

/** @fn char* httpsPOST(const char* url, const char* data, const char*
 * cert_path)
 * @brief forks and does a https POST request
 * @param url the request url
 * @param cert_path the path to the SSL certs
 * @param data the data to be posted
 * @return a pointer to the response. Has to be freed after usage. If the Https
 * call failed, NULL is returned.
 */
char* httpsPOST(const char* url, const char* data, struct curl_slist* headers,
                const char* cert_path, const char* username,
                const char* password) {
  struct pipeSet pipes = ipc_pipe_init();
  if (pipes.pipe1.rx == -1) {
    return NULL;
  }
  pid_t pid = fork();
  if (pid == -1) {
    syslog(LOG_AUTHPRIV | LOG_ALERT, "fork %m");
    oidc_setErrnoError();
    return NULL;
  }
  if (pid == 0) {  // child
    struct ipcPipe childPipes = toClientPipes(pipes);
    char* res = _httpsPOST(url, data, headers, cert_path, username, password);
    handleChild(res, childPipes);
    return NULL;
  } else {  // parent
    struct ipcPipe parentPipes = toServerPipes(pipes);
    return _handleParent(parentPipes);
  }
}

char* sendPostDataWithBasicAuth(const char* endpoint, const char* data,
                                const char* cert_path, const char* username,
                                const char* password) {
  return httpsPOST(endpoint, data, NULL, cert_path, username, password);
}

char* sendPostDataWithoutBasicAuth(const char* endpoint, const char* data,
                                   const char* cert_path) {
  return httpsPOST(endpoint, data, NULL, cert_path, NULL, NULL);
}
