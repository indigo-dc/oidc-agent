#include "httpserver.h"
#include "oidc_error.h"
#include "oidc_utilities.h"
#include "api.h"
#include "ipc.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <syslog.h>
#include <strings.h>

#define NO_CODE "Code param not found!"
#define WRONG_STATE "State comparison failed!"

char* communicateWithPath(char* fmt, ...) {
  va_list args;
  va_start(args, fmt);

  static struct connection con;
  if(ipc_initWithPath(&con)!=OIDC_SUCCESS) { 
    return NULL; 
  }
  if(ipc_connect(con)<0) {
    return NULL;
  }
  ipc_vwrite(*(con.sock), fmt, args);
  char* response = ipc_read(*(con.sock));
  ipc_close(&con);
  if(NULL==response) {
    fprintf(stderr, "An unexpected error occured. It seems that oidc-agent has stopped.\n%s\n", oidc_serror());
    exit(EXIT_FAILURE);
  }
  return response;
}

static int ahc_echo(void* cls,
    struct MHD_Connection * connection,
    const char * url,
    const char * method,
    const char * version,
    const char * upload_data __attribute__((unused)),
    size_t * upload_data_size,
    void ** ptr) {
  static int dummy;
  struct MHD_Response * response;
  int ret;
  char* res = NULL;

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
  const char* code = MHD_lookup_connection_value (connection, 
      MHD_GET_ARGUMENT_KIND,
      "code"); 
  const char* state = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "state");
  if(code) {
    syslog(LOG_AUTHPRIV|LOG_DEBUG, "HttpServer: Code is %s", code);
    char** cr = (char**) cls;
    if(strcmp(cr[2], state)==0) {
      res = communicateWithPath(REQUEST_CODEEXCHANGE, cr[0], cr[1], code, state);
      char* oidcgen_call = oidc_sprintf(REQUEST_CODEEXCHANGE, cr[0], cr[1], code, state);
      if(res==NULL) {
        char* info = "An error occured during codeExchange. Please try calling oidc-gen with the following command: \noidc-gen --codeExchangeRequest='%s'";
        res = oidc_sprintf(info, oidcgen_call);
        response = MHD_create_response_from_buffer (strlen(res),
            (void*) res,
            MHD_RESPMEM_MUST_FREE);
        ret = MHD_queue_response(connection,
            MHD_HTTP_OK,
            response);
      } else {
        syslog(LOG_AUTHPRIV|LOG_DEBUG, "Httpserver ipc response is: %s", res);
        clearFreeString(res);
        char* fmt = "Successfully performed code exchange. oidc-gen should have received the generated account config. Please check back with oidc-gen. If there is no success message, please call oidc-gen again with the following command: \noidc-gen --state='%s'";
        res = oidc_sprintf(fmt, cr[2]);
        response = MHD_create_response_from_buffer (strlen(res),
            (void*) res,
            MHD_RESPMEM_MUST_FREE); // Note that MHD just frees the data and does not use clearFree
        MHD_add_response_header(response, MHD_HTTP_HEADER_CONTENT_TYPE, "application/json");
        ret = MHD_queue_response(connection,
            MHD_HTTP_OK,
            response);
      }
      clearFreeString(cr[0]);
      clearFreeString(cr[1]);
      clearFreeString(cr[2]);
      clearFree(cr, sizeof(char*)*3);
      *ptr = "shutdown";
      clearFreeString(oidcgen_call);
    } else {
      response = MHD_create_response_from_buffer(strlen(WRONG_STATE), (void*) WRONG_STATE, MHD_RESPMEM_PERSISTENT);
      ret = MHD_queue_response(connection,
          MHD_HTTP_BAD_REQUEST,
          response);
    }
  } else {
    response = MHD_create_response_from_buffer(strlen(NO_CODE), (void*) NO_CODE, MHD_RESPMEM_PERSISTENT);
    ret = MHD_queue_response(connection,
        MHD_HTTP_BAD_REQUEST,
        response);
  }
  MHD_destroy_response(response);
  return ret;
}

void requestCompletedCallback (void *cls, struct MHD_Connection* connection __attribute__((unused)), void **con_cls, enum MHD_RequestTerminationCode toe) {
  if(toe == MHD_REQUEST_TERMINATED_COMPLETED_OK && strcmp("shutdown", (char*) *con_cls)==0) {
    struct MHD_Daemon** d_ptr = cls;
    stopHttpServer(d_ptr);
  }
}

void panicCallback(void *cls __attribute__((unused)), const char *file, unsigned int line, const char *reason){
  if(reason && (strcasecmp(reason, "Failed to join a thread\n") == 0)) {

  } else {
    syslog(LOG_AUTHPRIV|LOG_ALERT, "Fatal error in GNU libmicrohttpd %s:%d: %s", file, line, reason);
    fprintf(stderr, "Fatal error in GNU libmicrohttpd %s:%d: %s", file, line, reason);
    abort();
  }
}

/**
 * @param config a pointer to a json account config.
 * */
struct MHD_Daemon** startHttpServer(unsigned short port, char* config, char* state) {
  MHD_set_panic_func(&panicCallback, NULL);
  struct MHD_Daemon** d_ptr = calloc(sizeof(struct MHD_Daemon*),1);
  const char** cls = calloc(sizeof(char*), 3);
  cls[0] = oidc_sprintf("%s", config);
  cls[1] = portToUri(port);
  cls[2] = oidc_sprintf("%s", state);
  *d_ptr = MHD_start_daemon(MHD_USE_THREAD_PER_CONNECTION,
      port,
      NULL,
      NULL,
      &ahc_echo,
      cls,
      MHD_OPTION_NOTIFY_COMPLETED, &requestCompletedCallback, d_ptr,
      MHD_OPTION_END);
  if (*d_ptr == NULL) {
    syslog(LOG_AUTHPRIV|LOG_ERR, "Error starting the HttpServer");
    oidc_errno = OIDC_EHTTPD;
    return NULL;
  }
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "HttpServer: Started HttpServer");
  return d_ptr;
}

void stopHttpServer(struct MHD_Daemon** d_ptr) {
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "HttpServer: Stopping HttpServer");
  MHD_stop_daemon(*d_ptr);
  clearFree(d_ptr, sizeof(struct MHD_Daemon*));
}


