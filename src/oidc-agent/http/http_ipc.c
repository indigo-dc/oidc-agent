#define _GNU_SOURCE
#include "http_ipc.h"
#include "ipc/cryptCommunicator.h"
#include "ipc/cryptIpc.h"
#include "utils/oidc_error.h"

#include <fcntl.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <syslog.h>
#include <unistd.h>

char* _handleParent(int fd[2]) {
  close(fd[1]);
  char* e = server_ipc_read(
      fd[0]);  // TODO this is a pipe, we cannot read and write to it, we need
               // to do this correctly, two pipes and ipc_pipe_crypt functions
  close(fd[0]);
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

void handleChild(char* res, int fd) {
  if (res != NULL) {
    struct ipc_keySet* ipc_keys = client_ipc_writeToSock(fd, res);
    if (ipc_keys == NULL) {
      syslog(LOG_AUTHPRIV | LOG_ERR, "%s", oidc_serror());
    }
    secFreeIpcKeySet(ipc_keys);
    close(fd);
    exit(EXIT_SUCCESS);
  }
  struct ipc_keySet* ipc_keys = client_ipc_writeToSock(fd, res);
  if (ipc_keys == NULL) {
    syslog(LOG_AUTHPRIV | LOG_ERR, "%s", oidc_serror());
  }
  secFreeIpcKeySet(ipc_keys);
  close(fd);
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
  int fd[2];
  if (pipe2(fd, O_DIRECT) != 0) {
    oidc_setErrnoError();
    return NULL;
  }
  pid_t pid = fork();
  if (pid == -1) {
    syslog(LOG_AUTHPRIV | LOG_ALERT, "fork %m");
    oidc_setErrnoError();
    return NULL;
  }
  if (pid == 0) {  // child
    close(fd[0]);
    char* res = _httpsGET(url, headers, cert_path);
    handleChild(res, fd[1]);
    return NULL;
  } else {  // parent
    signal(SIGCHLD, SIG_IGN);
    return _handleParent(fd);
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
  int fd[2];
  if (pipe2(fd, O_DIRECT) != 0) {
    oidc_setErrnoError();
    return NULL;
  }
  pid_t pid = fork();
  if (pid == -1) {
    syslog(LOG_AUTHPRIV | LOG_ALERT, "fork %m");
    oidc_setErrnoError();
    return NULL;
  }
  if (pid == 0) {  // child
    close(fd[0]);
    char* res = _httpsPOST(url, data, headers, cert_path, username, password);
    handleChild(res, fd[1]);
    return NULL;
  } else {  // parent
    return _handleParent(fd);
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
