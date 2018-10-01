#ifndef HTTP_ERRORHANDLER_H
#define HTTP_ERRORHANDLER_H

#include "../oidc_error.h"
#include <curl/curl.h>

oidc_error_t CURLErrorHandling(int res, CURL* curl);

#endif // HTTP_ERRORHANDLER_H
