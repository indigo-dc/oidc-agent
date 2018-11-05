#ifndef HTTP_REQUEST_HANDLER_H
#define HTTP_REQUEST_HANDLER_H

#include <microhttpd.h>

int request_echo(void* cls, struct MHD_Connection* connection, const char* url,
                 const char* method, const char* version,
                 const char* upload_data, size_t* upload_data_size, void** ptr);

#endif  // HTTP_REQUEST_HANDLER_H
