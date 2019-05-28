#define _XOPEN_SOURCE

#include "requestHandler.h"

#include "defines/ipc_values.h"
#include "ipc/serveripc.h"
#include "oidc-agent/oidc/parse_oidp.h"
#include "utils/errorUtils.h"
#include "utils/logger.h"
#include "utils/memory.h"
#include "utils/stringUtils.h"

#include <signal.h>
#include <string.h>
#include <sys/types.h>

const char* const HTML_SUCCESS =
#include "static/success.html"
    ;
const char* const HTML_NO_CODE =
#include "static/no_code.html"
    ;
const char* const HTML_WRONG_STATE =
#include "static/wrong_state.html"
    ;
const char* const HTML_CODE_EXCHANGE_FAILED =
#include "static/code_exchange_failed.html"
    ;
const char* const HTML_CODE_EXCHANGE_FAILED_WITH_ERROR =
#include "static/code_exchange_failed_with_error.html"
    ;
const char* const HTML_ERROR =
#include "static/error.html"
    ;

static int makeResponseCodeExchangeFailed(struct MHD_Connection* connection,
                                          const char*            url) {
  char*                res      = oidc_sprintf(HTML_CODE_EXCHANGE_FAILED, url);
  struct MHD_Response* response = MHD_create_response_from_buffer(
      strlen(res), (void*)res, MHD_RESPMEM_MUST_COPY);
  secFree(res);
  int ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
  MHD_destroy_response(response);
  return ret;
}

static int makeResponseFromIPCResponse(struct MHD_Connection* connection,
                                       char* res, const char* url,
                                       const char* state) {
  char* error = parseForError(res);
  if (error) {
    res = oidc_sprintf(HTML_CODE_EXCHANGE_FAILED_WITH_ERROR, error, url);
    secFree(error);
  } else {
    res = oidc_sprintf(HTML_SUCCESS, state);
  }
  struct MHD_Response* response = MHD_create_response_from_buffer(
      strlen(res), (void*)res, MHD_RESPMEM_MUST_COPY);
  secFree(res);
  int ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
  MHD_destroy_response(response);
  return ret;
}

static int makeResponseWrongState(struct MHD_Connection* connection) {
  struct MHD_Response* response = MHD_create_response_from_buffer(
      strlen(HTML_WRONG_STATE), (void*)HTML_WRONG_STATE,
      MHD_RESPMEM_PERSISTENT);
  int ret = MHD_queue_response(connection, MHD_HTTP_BAD_REQUEST, response);
  MHD_destroy_response(response);
  return ret;
}

static int makeResponseError(struct MHD_Connection* connection) {
  const char* error =
      MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "error");
  const char* error_description = MHD_lookup_connection_value(
      connection, MHD_GET_ARGUMENT_KIND, "error_description");
  struct MHD_Response* response;
  if (error) {
    char* err = combineError(error, error_description);
    logger(ERROR, "HttpServer Error: %s", err);
    char* res = oidc_sprintf(HTML_ERROR, err);
    secFree(err);
    response = MHD_create_response_from_buffer(strlen(res), (void*)res,
                                               MHD_RESPMEM_MUST_COPY);
    secFree(res);
    kill(getpid(), SIGTERM);
  } else {
    response = MHD_create_response_from_buffer(
        strlen(HTML_NO_CODE), (void*)HTML_NO_CODE, MHD_RESPMEM_PERSISTENT);
  }
  int ret = MHD_queue_response(connection, MHD_HTTP_BAD_REQUEST, response);
  MHD_destroy_response(response);
  return ret;
}

static int handleRequest(void* cls, struct MHD_Connection* connection) {
  const char* code =
      MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "code");
  const char* state =
      MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "state");

  if (code == NULL) {
    return makeResponseError(connection);
  }
  logger(DEBUG, "HttpServer: Code is %s", code);
  char** cr = (char**)cls;
  if (!strequal(cr[1], state)) {
    return makeResponseWrongState(connection);
  }
  char* url = oidc_sprintf("%s?code=%s&state=%s", cr[0], code, state);
  char* res = ipc_cryptCommunicateWithServerPath(REQUEST_CODEEXCHANGE, url);
  int   ret;
  if (res == NULL) {
    ret = makeResponseCodeExchangeFailed(connection, url);
  } else {
    logger(DEBUG, "Httpserver ipc response is: %s", res);
    ret = makeResponseFromIPCResponse(connection, res, url, state);
  }
  secFree(url);
  secFreeArray(cr, 2);
  return ret;
}

int request_echo(void* cls, struct MHD_Connection* connection, const char* url,
                 const char* method, const char* version,
                 const char* upload_data __attribute__((unused)),
                 size_t* upload_data_size, void** ptr) {
  static int dummy;

  logger(DEBUG, "HttpServer: New connection: %s %s %s", version, method, url);

  if (!strequal(method, "GET")) {
    return MHD_NO; /* unexpected method */
  }
  if (&dummy != *ptr) {
    /* The first time only the headers are valid,
       do not respond in the first round... */
    *ptr = &dummy;
    return MHD_YES;
  }
  if (0 != *upload_data_size) {
    return MHD_NO; /* upload data in a GET!? */
  }
  *ptr = NULL; /* clear context pointer */

  return handleRequest(cls, connection);
}
