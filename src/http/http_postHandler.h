#ifndef HTTP_POSTHANDLER_H
#define HTTP_POSTHANDLER_H

#include <curl/curl.h>

void setPostData(CURL* curl, const char* data);

#endif  // HTTP_POSTHANDLER_H
