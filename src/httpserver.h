#ifndef HTTPSERVER_H
#define HTTPSERVER_H

#include <microhttpd.h>

#define HTTP_DEFAULT_PORT 2912
#define HTTP_FALLBACK_PORT 8080

static const char* const HTML_SUCCESS =  
#include "static/success.html" 
;
static const char* const HTML_NO_CODE =
#include "static/no_code.html"
;
static const char* const HTML_WRONG_STATE = 
#include "static/wrong_state.html"
;
static const char* const HTML_CODE_EXCHANGE_FAILED = 
#include "static/code_exchange_failed.html"
;
static const char* const HTML_CODE_EXCHANGE_FAILED_WITH_ERROR = 
#include "static/code_exchange_failed_with_error.html"
;
static const char* const HTML_ERROR =
#include "static/error.html"
;

struct MHD_Daemon** startHttpServer(unsigned short port, char* config, char* state) ;
void stopHttpServer(struct MHD_Daemon** d) ;

#endif // HTTPSERVER_H
