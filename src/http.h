#ifndef HTTP_H
#define HTTP_H


struct string {
  char *ptr;
  size_t len;
};

const char* httpsGET(const char* url) ;
const char* httpsPOST(const char* url, const char data[]) ;

#endif
