#define _XOPEN_SOURCE

#include "requestHandler.h"

#include "../parse_oidp.h"
#include "../utils/cleaner.h"
#include "../ipc/ipc_values.h"
#include "../ipc/communicator.h"
#include "../utils/errorUtils.h"
#include "../utils/stringUtils.h"

#include <syslog.h>
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

static int makeResponseCodeExchangeFailed(struct MHD_Connection* connection, char* oidcgen_call) {
  char* res = oidc_sprintf(HTML_CODE_EXCHANGE_FAILED, oidcgen_call);
  struct MHD_Response* response = MHD_create_response_from_buffer (strlen(res), (void*) res, MHD_RESPMEM_MUST_FREE);
  int ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
  MHD_destroy_response(response);
  return ret;
}

static int makeResponseFromIPCResponse(struct MHD_Connection* connection, char* res, char* oidcgen_call, const char* state) {
  char* error = parseForError(res);
  if(error) {
    res = oidc_sprintf(HTML_CODE_EXCHANGE_FAILED_WITH_ERROR, error, oidcgen_call);
    clearFreeString(error);
  } else {
    res = oidc_sprintf(HTML_SUCCESS, state);
  }
  struct MHD_Response* response = MHD_create_response_from_buffer (strlen(res), (void*) res, MHD_RESPMEM_MUST_FREE); // Note that MHD just frees the data and does not use clearFree
  int ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
  MHD_destroy_response(response);
  return ret;
}

static int makeResponseWrongState(struct MHD_Connection* connection) {
  struct MHD_Response* response = MHD_create_response_from_buffer(strlen(HTML_WRONG_STATE), (void*) HTML_WRONG_STATE, MHD_RESPMEM_PERSISTENT);
  int ret = MHD_queue_response(connection, MHD_HTTP_BAD_REQUEST, response);
  MHD_destroy_response(response);
  return ret;
}

static int makeResponseError(struct MHD_Connection* connection) {
  const char* error = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "error");
  const char* error_description = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "error_description");
  struct MHD_Response* response;
  if(error) {
    char* err = combineError(error, error_description);
    syslog(LOG_AUTHPRIV|LOG_ERR, "HttpServer Error: %s", err);
    char* res = oidc_sprintf(HTML_ERROR, err);
    clearFreeString(err);
    response = MHD_create_response_from_buffer(strlen(res), (void*) res, MHD_RESPMEM_MUST_FREE);
    kill(getpid(), SIGTERM);
  } else {
    response = MHD_create_response_from_buffer(strlen(HTML_NO_CODE), (void*) HTML_NO_CODE, MHD_RESPMEM_PERSISTENT);
  }
  int ret = MHD_queue_response(connection, MHD_HTTP_BAD_REQUEST, response);
  MHD_destroy_response(response);
  return ret;
}

static int handleRequest(void* cls, struct MHD_Connection* connection) {
  const char* code = MHD_lookup_connection_value (connection, MHD_GET_ARGUMENT_KIND, "code"); 
  const char* state = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "state");

  if(code == NULL) {
    return makeResponseError(connection);
  }     
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "HttpServer: Code is %s", code);
  char** cr = (char**) cls;
  if(strcmp(cr[2], state)!=0) {
    return makeResponseWrongState(connection);
  }
  char* res = ipc_communicateWithPath(REQUEST_CODEEXCHANGE, cr[0], cr[1], code, state);
  char* oidcgen_call = oidc_sprintf(REQUEST_CODEEXCHANGE, cr[0], cr[1], code, state);
  int ret;
  if(res == NULL) {
    ret = makeResponseCodeExchangeFailed(connection, oidcgen_call);
  } else {
    syslog(LOG_AUTHPRIV|LOG_DEBUG, "Httpserver ipc response is: %s", res);
    ret = makeResponseFromIPCResponse(connection, res, oidcgen_call, state);
  }
  clearFreeStringArray(cr, 3);
  clearFreeString(oidcgen_call);
  return ret;
}

int request_echo(void* cls,
    struct MHD_Connection* connection,
    const char* url,
    const char* method,
    const char* version,
    const char* upload_data __attribute__((unused)),
    size_t* upload_data_size,
    void** ptr) {

  static int dummy;

  syslog(LOG_AUTHPRIV|LOG_DEBUG, "HttpServer: New connection: %s %s %s", version, method, url);

  if (0 != strcmp(method, "GET")) {
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
