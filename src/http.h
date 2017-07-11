#ifndef HTTP_H
#define HTTP_H


struct string {
  char *ptr;
  size_t len;
};

const char* httpsGET(const char* url) ;
const char* httpsPOST(const char* url, const char data[]) ;
static size_t write_callback(void *ptr, size_t size, size_t nmemb, struct string *s);
static size_t read_callback(void *ptr, size_t size, size_t nmemb, struct string *pooh);
void init_string(struct string *s) ;

#endif
