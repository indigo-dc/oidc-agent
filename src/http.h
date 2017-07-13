#ifndef HTTP_H
#define HTTP_H


struct string {
  char *ptr;
  size_t len;
};

char* httpsGET(const char* url) ;
char* httpsPOST(const char* url, const char data[]) ;

#endif
