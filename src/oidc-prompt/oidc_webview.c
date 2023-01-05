#define _XOPEN_SOURCE 700
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
  strreplace_inplace(str, "\\n", '\n');   \
  strreplace_inplace(str, "\\\"", '"');

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

void webview(const char* title, const char* html, int w_pc, int h_pc) {
  if (html == NULL) {
    return;
  }
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
char* readFile(const char* path) {
  FILE* fp = fopen(path, "rb");
  if (!fp) {
    return NULL;
  }

  if (fseek(fp, 0L, SEEK_END) != 0) {
    return NULL;
  }
  long lSize = ftell(fp);
  rewind(fp);
  if (lSize < 0) {
    return NULL;
  }

  char* buffer = calloc(lSize + 1, 1);
  if (buffer == NULL) {
    return NULL;
  }

  if (1 != fread(buffer, lSize, 1, fp)) {
    free(buffer);
    return NULL;
  }
  fclose(fp);
  return buffer;
}

int main(int argc, char** argv) {
  if (argc < 3) {
    printf("Not enough arguments.\n");
    exit(EXIT_FAILURE);
  }
  const char* title = argv[1];
  const char* _path = argv[2];
  char*       path  = NULL;
  if (_path[0] == '/') {
    const char* home = getenv(HOME_ENV);
    path             = malloc(strlen(home) + 1 + strlen(_path) + 1);
    if (path == NULL) {
      return EXIT_FAILURE;
    }
    sprintf(path, "%s/%s", home, _path);
  }

  int w_pc = 0;
  int h_pc = 0;
  if (argc > 3 && strlen(argv[3]) > 0) {
    w_pc = atoi(argv[3]);
  }
  if (argc > 4 && strlen(argv[4]) > 0) {
    h_pc = atoi(argv[4]);
  }
  char* html = readFile(path ? path : _path);
  if (path) {
    free(path);
  }
  FILE* fp = freopen("nul", "r+", stderr);
  webview(title, html, w_pc, h_pc);
  free(html);
}
#endif
