#include "httpserver.h"
#include "oidc_error.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <syslog.h>

#define NO_CODE "Code param not found!"

static int ahc_echo(void * cls,
    struct MHD_Connection * connection,
    const char * url,
    const char * method,
    const char * version,
    const char * upload_data,
    size_t * upload_data_size,
    void ** ptr) {
  static int dummy;
  struct MHD_Response * response;
  int ret;

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
  if(code) {
    syslog(LOG_AUTHPRIV|LOG_DEBUG, "HttpServer: Code is %s", code);
    response = MHD_create_response_from_buffer (strlen(code),
        (void*) code,
        MHD_RESPMEM_PERSISTENT);
    ret = MHD_queue_response(connection,
        MHD_HTTP_OK,
        response);
    *ptr = "shutdown";
  } else {
    response = MHD_create_response_from_buffer(strlen(NO_CODE), (void*) NO_CODE, MHD_RESPMEM_PERSISTENT);
    ret = MHD_queue_response(connection,
        MHD_HTTP_BAD_REQUEST,
        response);
  }
  MHD_destroy_response(response);
  return ret;
}

void requestCompletedCallback (void *cls, struct MHD_Connection* connection, void **con_cls, enum MHD_RequestTerminationCode toe) {
  if(toe == MHD_REQUEST_TERMINATED_COMPLETED_OK && strcmp("shutdown", (char*) *con_cls)==0) {
    struct MHD_Daemon** d_ptr = cls;
    stopHttpServer(*d_ptr);
  }
}

struct MHD_Daemon* startHttpServer(unsigned short port) {
  struct MHD_Daemon** d_ptr = calloc(sizeof(struct MHD_Daemon*),1);
  *d_ptr = MHD_start_daemon(MHD_USE_THREAD_PER_CONNECTION,
      port,
      NULL,
      NULL,
      &ahc_echo,
      NULL,
      MHD_OPTION_NOTIFY_COMPLETED, &requestCompletedCallback ,d_ptr,
      MHD_OPTION_END);
  if (*d_ptr == NULL) {
    syslog(LOG_AUTHPRIV|LOG_ERR, "Error starting the HttpServer");
    oidc_errno = OIDC_EHTTPD;
    return NULL;
  }
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "HttpServer: Started HttpServer");
  return *d_ptr;
}

void stopHttpServer(struct MHD_Daemon* d) {
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "HttpServer: Stopping HttpServer");
  MHD_stop_daemon(d);
}


