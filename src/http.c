#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

#include "http.h"




void init_string(struct string *s) {
  s->len = 0;
  s->ptr = malloc(s->len+1);

  if (s->ptr == NULL) {
    fprintf(stderr, "malloc() failed\n");
    exit(EXIT_FAILURE);
  }
  s->ptr[0] = '\0';
}

size_t writefunc(void *ptr, size_t size, size_t nmemb, struct string *s) {
  size_t new_len = s->len + size*nmemb;
  s->ptr = realloc(s->ptr, new_len+1);

  if (s->ptr == NULL) {
    fprintf(stderr, "realloc() failed\n");
    exit(EXIT_FAILURE);
  }
  memcpy(s->ptr+s->len, ptr, size*nmemb);
  s->ptr[new_len] = '\0';
  s->len = new_len;

  return size*nmemb;
}

const char* httpsGET(const char* url) {
  CURL* curl;
  CURLcode res;

  curl_global_init(CURL_GLOBAL_ALL);

  curl = curl_easy_init();
  struct string s;
  if(curl) {
    init_string(&s);

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writefunc);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &s);

#ifdef SKIP_PEER_VERIFICATION
    /*
     * If you want to connect to a site who isn't using a certificate that is
     * signed by one of the certs in the CA bundle you have, you can skip the
     * verification of the server's certificate. This makes the connection
     * A LOT LESS SECURE.
     *
     * If you have a CA cert for the server stored someplace else than in the
     * default bundle, then the CURLOPT_CAPATH option might come handy for
     * you.
     */ 
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
#endif

#ifdef SKIP_HOSTNAME_VERIFICATION
    /*
     * If the site you're connecting to uses a different host name that what
     * they have mentioned in their server certificate's commonName (or
     * subjectAltName) fields, libcurl will refuse to connect. You can skip
     * this check, but this will make the connection less secure.
     */ 
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
#endif

    res = curl_easy_perform(curl);
    if(res != CURLE_OK)
      fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));

    curl_easy_cleanup(curl);  
    printf("%s\n\n",s);
  } 
  else {
    fprintf(stderr, "Couldn't init curl.\n");
    exit(EXIT_FAILURE);
  }
  curl_global_cleanup();
  return s.ptr;
}

