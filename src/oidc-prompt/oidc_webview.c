#define _XOPEN_SOURCE 500
#include "oidc_webview.h"

#define WEBVIEW_HEADER

#include <stddef.h>
#include <stdio.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <stdlib.h>

#include "utils/logger.h"
#include "utils/memory.h"
#include "utils/string/stringUtils.h"
#endif

#include "defines/settings.h"
#include "webview.h"

void strreplace_inplace(char* str, const char* old, const char new) {
  if (str == NULL || old == NULL) {
    return;
  }

  size_t old_len = strlen(old);
  char*  pos     = strstr(str, old);
  while (pos != NULL) {
    *pos = new;
    memmove(pos + 1, pos + old_len, strlen(pos + old_len) + 1);
    pos = strstr(pos, old);
  }
}

#define EXTRACT_SINGLE_INT_ARG_FROM_REQ  \
  int i = 0;                             \
  if (req != NULL && strlen(req) >= 3) { \
    i = atoi(req + 1);                   \
  }

#define EXTRACT_SINGLE_ARG_FROM_REQ       \
  if (req == NULL || strlen(req) < 4) {   \
    return;                               \
  }                                       \
  char* str            = strdup(req + 2); \
  str[strlen(str) - 2] = '\0';            \
  strreplace_inplace(str, "\\n", '\n');

#ifdef _WIN32
void print(const char* seq, const char* req, void* arg) {
#else
void print(const char* seq __attribute__((unused)), const char* req,
           void* arg __attribute__((unused))) {
#endif
  EXTRACT_SINGLE_ARG_FROM_REQ
  printf("%s\n", str);
  free(str);
}
#ifdef _WIN32
void terminate(const char* seq, const char* req, void* arg) {
#else
void terminate(const char* seq __attribute__((unused)), const char* req,
               void* arg __attribute__((unused))) {
#endif
  EXTRACT_SINGLE_INT_ARG_FROM_REQ
  exit(i);
}
#ifdef _WIN32
void openLink(const char* seq, const char* req, void* arg) {
#else
void openLink(const char* seq __attribute__((unused)), const char* req,
              void* arg __attribute__((unused))) {
#endif
  EXTRACT_SINGLE_ARG_FROM_REQ
#ifdef _WIN32
  ShellExecute(NULL, URL_OPENER, str, NULL, NULL, SW_SHOWNORMAL);
#else
  char* cmd = oidc_sprintf(URL_OPENER " \"%s\"", str);
  if (system(cmd) != 0) {
    logger(NOTICE, "Cannot open url");
  }
  secFree(cmd);
#endif
  free(str);
}

#include <ctype.h>
char rfc3986[256] = {0};
char html5[256]   = {0};
void url_encoder_rfc_tables_init() {
  for (int i = 0; i < 256; i++) {
    rfc3986[i] =
        isalnum(i) || i == '~' || i == '-' || i == '.' || i == '_' ? i : 0;
    html5[i] = isalnum(i) || i == '*' || i == '-' || i == '.' || i == '_' ? i
               : (i == ' ')                                               ? '+'
                                                                          : 0;
  }
}
char* url_encode(const char* table, const unsigned char* s, char* enc) {
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
void webview_set_html(webview_t w, const char* html) {
  const char* const prefix  = "data:text/html,";
  char*             html_en = malloc(3 * strlen(html) + 1);
  url_encoder_rfc_tables_init();
  url_encode(html5, (const unsigned char*)html, html_en);
  char* url = malloc(strlen(prefix) + strlen(html_en) + 1);
  strcpy(url, prefix);
  strcat(url, html_en);
  free(html_en);
  webview_navigate(w, url);
  free(url);
}

void webview(const char* title, const char* html, int w_pc, int h_pc) {
  int width  = 480;
  int height = 320;
  if (w_pc) {
    width *= w_pc;
    width /= 100;
  }
  if (h_pc) {
    height *= h_pc;
    height /= 100;
  }
  webview_t w = webview_create(1, NULL);
  webview_set_title(w, title ? title : "oidc-prompt");
  webview_set_size(w, width, height, WEBVIEW_HINT_NONE);
  webview_bind(w, "terminate", terminate, NULL);
  webview_bind(w, "print", print, NULL);
  webview_bind(w, "openLink", openLink, NULL);
  webview_set_html(w, html);
  webview_run(w);
  webview_destroy(w);
}

#ifdef _WIN32
int main(int argc, char** argv) {
  if (argc < 3) {
    printf("Not enough arguments.\n");
    exit(EXIT_FAILURE);
  }
  const char* title = argv[1];
  const char* html  = argv[2];
  webview(title, html);
}
#endif
