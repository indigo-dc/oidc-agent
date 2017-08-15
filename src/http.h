#ifndef HTTP_H
#define HTTP_H

char* httpsGET(const char* url, const char* cert_path) ;
char* httpsPOST(const char* url, const char data[], const char* cert_path) ;

#endif
