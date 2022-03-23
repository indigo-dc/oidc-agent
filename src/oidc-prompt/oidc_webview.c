#include "oidc_webview.h"

#define WEBVIEW_HEADER

#include <stddef.h>
#include <stdio.h>
#include <string.h>

#ifdef WIN32
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

#define EXTRACT_SINGLE_ARG_FROM_REQ     \
  if (req == NULL || strlen(req) < 4) { \
    return;                             \
  }                                     \
  req += 2; /* cut [" */                \
  char str[strlen(req) + 1];            \
  strcpy(str, req);                     \
  str[strlen(req) - 2] = '\0';          \
  strreplace_inplace(str, "\\n", '\n');

void print(const char* seq __attribute__((unused)), const char* req,
           void* arg __attribute__((unused))) {
  EXTRACT_SINGLE_ARG_FROM_REQ
  printf("%s\n", str);
}
void terminate(const char* seq __attribute__((unused)), const char* req,
               void* arg __attribute__((unused))) {
  EXTRACT_SINGLE_INT_ARG_FROM_REQ
  exit(i);
}
void openLink(const char* seq __attribute__((unused)), const char* req,
              void* arg __attribute__((unused))) {
  EXTRACT_SINGLE_ARG_FROM_REQ
#ifdef WIN32
  ShellExecute(NULL, URL_OPENER, str, NULL, NULL, SW_SHOWNORMAL);
#else
  char* cmd = oidc_sprintf(URL_OPENER " \"%s\"", str);
  if (system(cmd) != 0) {
    logger(NOTICE, "Cannot open url");
  }
  secFree(cmd);
#endif
}

void webview(const char* title, const char* html) {
  webview_t w = webview_create(0, NULL);
  webview_set_title(w, title ?: "oidc-prompt");
  webview_set_size(w, 480, 320, WEBVIEW_HINT_NONE);
  webview_bind(w, "terminate", terminate, NULL);
  webview_bind(w, "print", print, NULL);
  webview_bind(w, "openLink", openLink, NULL);
  webview_set_html(w, html);
  webview_run(w);
  webview_destroy(w);
}
