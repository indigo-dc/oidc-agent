#include "oidc-prompt.h"

#define WEBVIEW_HEADER
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "defines/settings.h"
#include "mustache.h"
#include "utils/crypt/crypt.h"
#include "utils/disableTracing.h"
#include "utils/file_io/file_io.h"
#include "utils/json.h"
#include "utils/listUtils.h"
#include "utils/logger.h"
#include "utils/memory.h"
#include "utils/printer.h"
#include "utils/string/stringUtils.h"
#include "webview.h"

void print(const char* seq __attribute__((unused)), const char* req,
           void* arg __attribute__((unused))) {
  char* str = JSONArrayStringToDelimitedString(req, "\n");
  printf("%s\n", str);
  secFree(str);
}
void terminate(const char* seq __attribute__((unused)), const char* req,
               void* arg __attribute__((unused))) {
  char* str = JSONArrayStringToDelimitedString(req, "");
  int   r   = strToInt(str);
  secFree(str);
  exit(r);
}
void openLink(const char* seq __attribute__((unused)), const char* req,
              void* arg __attribute__((unused))) {
  char* uri = JSONArrayStringToDelimitedString(req, " ");
  if (uri == NULL) {
    oidc_perror();
    return;
  }
#ifdef __MSYS__
  ShellExecute(NULL, URL_OPENER, uri, NULL, NULL, SW_SHOWNORMAL);
#else
  char* cmd = oidc_sprintf(URL_OPENER " \"%s\"", uri);
  if (system(cmd) != 0) {
    logger(NOTICE, "Cannot open url");
  }
  secFree(cmd);
#endif
  secFree(uri);
}

#if defined WIN32 || defined MSYS
int WINAPI WinMain(HINSTANCE hInt, HINSTANCE hPrevInst, LPSTR lpCmdLine,
                   int nCmdShow) {
#else
int main(int argc, char** argv) {
#endif
  platform_disable_tracing();
  logger_open("oidc-prompt");
  logger_setloglevel(NOTICE);

  struct arguments arguments;
  initArguments(&arguments);
  argp_parse(&argp, argc, argv, 0, 0, &arguments);

  cJSON* data = generateJSONObject(
      "title", cJSON_String, arguments.title, "text", cJSON_String,
      arguments.text, "label", cJSON_String, arguments.label ?: "", "init",
      cJSON_String, arguments.init ?: "", NULL);
  if (arguments.timeout != 0) {
    data = jsonAddNumberValue(data, "timeout", arguments.timeout);
  }
  char*       html        = NULL;
  const char* prompt_type = arguments.req_type;
  if (strequal(prompt_type, "password")) {
    html = mustache_main(SITE_PASSWORD, data);
  } else if (strequal(prompt_type, "input")) {
    html = mustache_main(SITE_INPUT, data);
  } else if (strequal(prompt_type, "confirm") ||
             strequal(prompt_type, "confirm-default-yes")) {
    data = jsonAddNumberValue(data, "yes-auto-focus", 1);
    html = mustache_main(SITE_CONFIRM, data);
  } else if (strequal(prompt_type, "confirm-default-no")) {
    data = jsonAddNumberValue(data, "no-auto-focus", 1);
    html = mustache_main(SITE_CONFIRM, data);
  } else if (strequal(prompt_type, "multiple")) {
    char* other = listToDelimitedString(arguments.additional_args, "\n");
    char* init  = other ? oidc_sprintf("%s\n%s", arguments.init, other)
                        : oidc_strcopy(arguments.init);
    secFree(other);
    setJSONValue(data, "init", init);
    data = jsonAddNumberValue(
        data, "rows",
        (arguments.additional_args ? arguments.additional_args->len : 0) + 2);
    html = mustache_main(SITE_MULTIPLE, data);
  } else if (strstarts(prompt_type, "select")) {
    if (strequal(prompt_type, "select-other")) {
      data = jsonAddNumberValue(data, "other", 1);
    }
    if (arguments.additional_args != NULL) {
      data = jsonAddJSON(data, "options",
                         listToJSONArray(arguments.additional_args));
    }
    html = mustache_main(SITE_SELECT, data);
  } else if (strstarts(prompt_type, "link")) {
    if (strValid(arguments.init)) {
      char*  iData = NULL;
      size_t size;
      if (readBinaryFile(arguments.init, &iData, &size) != OIDC_SUCCESS) {
        secFree(iData);
        oidc_perror();
        exit(oidc_errno);
      }
      char* base64 = toBase64(iData, size);
      secFree(iData);
      char* imgData =
          oidc_sprintf("data:image/%s;base64,%s", "svg+xml", base64);
      secFree(base64);
      data = jsonAddStringValue(data, "img-data", imgData);
    }
    html = mustache_main(SITE_LINK, data);
  } else {
    printError("Unknown prompt type '%s'\n", arguments.req_type);
    exit(EXIT_FAILURE);
  }

  webview_t w = webview_create(0, NULL);
  webview_set_title(w, arguments.title);
  webview_set_size(w, 480, 320, WEBVIEW_HINT_NONE);
  webview_bind(w, "terminate", terminate, NULL);
  webview_bind(w, "print", print, NULL);
  webview_bind(w, "openLink", openLink, NULL);
  secFreeJson(data);
  webview_set_html(w, html);
  secFree(html);
  webview_run(w);
  webview_destroy(w);
  return 0;
}