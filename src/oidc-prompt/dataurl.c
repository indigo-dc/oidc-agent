
#include "dataurl.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include "utils/crypt/crypt.h"
#include "utils/memory.h"
#include "utils/string/stringUtils.h"

char          rfc3986[256] = {0};
char          html5[256]   = {0};
unsigned char init_done    = 0;

void url_encoder_rfc_tables_init() {
  if (init_done) {
    return;
  }
  for (int i = 0; i < 256; i++) {
    rfc3986[i] =
        isalnum(i) || i == '~' || i == '-' || i == '.' || i == '_' ? i : 0;
    html5[i] = isalnum(i) || i == '*' || i == '-' || i == '.' || i == '_' ? i
               : (i == ' ')                                               ? '+'
                                                                          : 0;
  }
  init_done = 1;
}

char* url_encode(const char* table, const unsigned char* s, char* enc) {
  url_encoder_rfc_tables_init();
  for (; *s; s++) {
    if (table[*s])
      *enc = table[*s];
    else
      sprintf(enc, "%%%02X", *s);
    while (*++enc)
      ;
  }
  return (enc);
}

char* _htmlToDataURL(const char* html, unsigned char base64) {
  char* data;
  if (base64) {
    data = toBase64(html, strlen(html));
  } else {
    char* html_en = secAlloc(3 * strlen(html) + 1);
    url_encode(html5, (const unsigned char*)html, html_en);
    data = html_en;
  }
  char* url =
      oidc_sprintf("data:text/html%s,%s", base64 ? ";base64" : "", data);
  secFree(data);
  return url;
}

char* htmlToDataURL(const char* html) { return _htmlToDataURL(html, 0); }
char* htmlToBase64DataURL(const char* html) { return _htmlToDataURL(html, 1); }
