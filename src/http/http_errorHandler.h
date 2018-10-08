#ifndef HTTP_ERRORHANDLER_H
#define HTTP_ERRORHANDLER_H

#include <curl/curl.h>
#include "../oidc_error.h"

oidc_error_t CURLErrorHandling(int res, CURL* curl);

#endif  // HTTP_ERRORHANDLER_H
