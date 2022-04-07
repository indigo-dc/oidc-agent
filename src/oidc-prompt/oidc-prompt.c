#include "oidc-prompt.h"

#include <stddef.h>
#include <stdlib.h>
#ifdef __MSYS__
#include <windows.h>

#include "utils/system_runner.h"
#endif

#include "dataurl.h"
#include "mustache.h"
#include "oidc_webview.h"
#include "utils/crypt/crypt.h"
#include "utils/disableTracing.h"
#include "utils/file_io/file_io.h"
#include "utils/json.h"
#include "utils/listUtils.h"
#include "utils/logger.h"
#include "utils/memory.h"
#include "utils/printer.h"
#include "utils/string/stringUtils.h"

int main(int argc, char** argv) {
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
  int         h_pc        = 0;
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
    } else {
      h_pc = 150;
    }
    html = mustache_main(SITE_LINK, data);
  } else {
    printError("Unknown prompt type '%s'\n", arguments.req_type);
    exit(EXIT_FAILURE);
  }
  secFreeJson(data);

#ifdef __MSYS__
  const char* tmpdir  = getenv("TEMP");
  char*       r       = randomString(8);
  char*       tmpFile = oidc_pathcat(tmpdir, r);
  secFree(r);
  writeFile(tmpFile, html);

  char* cmd = oidc_sprintf("oidc-webview \"%s\" \"%s\" %d %d", arguments.title,
                           tmpFile, 0, h_pc);
  secFree(tmpFile);
  char* out = getOutputFromCommand(cmd);
  secFree(cmd);
  if (out == NULL) {
    exit(oidc_errno);
  }
  lastChar(out) = '\0';
  printStdout("%s\n", out);
  secFree(out);
#else
  webview(arguments.title, html, 0, h_pc);
#endif
  secFree(html);
  return 0;
}