#ifndef HTTP_H
#define HTTP_H

//#define SKIP_HOSTNAME_VERIFICATION
#define SKIP_PEER_VERIFICATION


struct string {
  char *ptr;
  size_t len;
};

const char* httpsGET(const char* url) ;
size_t writefunc(void *ptr, size_t size, size_t nmemb, struct string *s);
void init_string(struct string *s) ;

#endif
