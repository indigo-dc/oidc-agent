#include "http_ipc.h"

#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

#include "ipc/pipe.h"
#include "utils/agentLogger.h"
#include "utils/memory.h"
#include "utils/oidc_error.h"

char* _handleParent(struct ipcPipe pipes) {
  char* e = ipc_readFromPipe(pipes);
  ipc_closePipes(pipes);
  if (e == NULL) {
    return NULL;
  }
  char* end   = NULL;
  int   error = (int)strtol(e, &end, 10);
  if (error) {
    secFree(e);
    oidc_errno = error;
    agent_log(ERROR, "Error from http request: %s", oidc_serror());
    return NULL;
  }
  if (*end != '\0') {
    char* res = e;
    agent_log(DEBUG, "Received response: %s", res);
    return res;
  }
  secFree(e);
  agent_log(ERROR, "Internal error: Http sent 0");
  oidc_errno = OIDC_EHTTP0;
  return NULL;
}

void handleChild(char* res, struct ipcPipe pipes) {
  if (res == NULL) {
    ipc_writeToPipe(pipes, "%d", oidc_errno);
    exit(EXIT_FAILURE);
  }
  ipc_writeToPipe(pipes, res);
  secFree(res);
  exit(EXIT_SUCCESS);
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
    agent_log(ALERT, "fork %m");
    oidc_setErrnoError();
    return NULL;
  }
  if (pid == 0) {  // child
    struct ipcPipe childPipes = toClientPipes(pipes);
    logger_open("oidc-agent.http");
    char* res = _httpsGET(url, headers, cert_path);
    handleChild(res, childPipes);
    return NULL;
  } else {  // parent
    signal(SIGCHLD, SIG_IGN);
    struct ipcPipe parentPipes = toServerPipes(pipes);
    return _handleParent(parentPipes);
  }
}

/** @fn char* httpsDELETE(const char* url, const char* cert_path)
 * @brief forks and does a https DELETE request
 * @param url the request url
 * @param cert_path the path to the SSL certs
 * @return a pointer to the response. Has to be freed after usage. If the Https
 * call failed, NULL is returned.
 */
char* httpsDELETE(const char* url, struct curl_slist* headers,
                  const char* cert_path, const char* bearer_token) {
  struct pipeSet pipes = ipc_pipe_init();
  if (pipes.pipe1.rx == -1) {
    return NULL;
  }
  pid_t pid = fork();
  if (pid == -1) {
    agent_log(ALERT, "fork %m");
    oidc_setErrnoError();
    return NULL;
  }
  if (pid == 0) {  // child
    struct ipcPipe childPipes = toClientPipes(pipes);
    logger_open("oidc-agent.http");
    char* res = _httpsDELETE(url, headers, cert_path, bearer_token);
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
    agent_log(ALERT, "fork %m");
    oidc_setErrnoError();
    return NULL;
  }
  if (pid == 0) {  // child
    struct ipcPipe childPipes = toClientPipes(pipes);
    logger_open("oidc-agent.http");
    headers   = curl_slist_append(headers, HTTP_HEADER_ACCEPT_JSON);
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
