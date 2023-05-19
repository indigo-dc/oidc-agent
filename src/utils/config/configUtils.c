#include "configUtils.h"

#include "defines/msys.h"
#include "defines/settings.h"
#include "utils/file_io/file_io.h"
#include "utils/file_io/oidc_file_io.h"
#include "utils/json.h"
#include "utils/listUtils.h"
#include "utils/printer.h"
#include "utils/string/stringUtils.h"

static char* readUserConfig() {
  const char* user_config = getenv(OIDC_USER_CONFIG_PATH_ENV_NAME);
  if (user_config == NULL) {
    list_t* lines = getLinesFromOidcFileWithoutComments("config");
    char*   data  = listToDelimitedString(lines, "");
    secFreeList(lines);
    return data;
  }
  if (strcaseequal("/dev/null", user_config)) {
    return NULL;
  }
  list_t* lines = getLinesFromFileWithoutComments(user_config);
  char*   data  = listToDelimitedString(lines, "");
  secFreeList(lines);
  return data;
}

static char* readGlobalConfig() {
  list_t* lines = getLinesFromFileWithoutComments(
#ifdef ANY_MSYS
      ETC_CONFIG_FILE()
#else
      ETC_CONFIG_FILE
#endif
  );
  char* data = listToDelimitedString(lines, "");
  secFreeList(lines);
  return data;
}

cJSON* readConfig() {
  char* global = readGlobalConfig();
  char* user   = readUserConfig();
  if (!strValid(global)) {
    secFree(global);
  }
  if (!strValid(user)) {
    secFree(user);
  }
  if (global == NULL && user == NULL) {
    return NULL;
  }
  cJSON* g = stringToJson(global);
  cJSON* u = stringToJson(user);
  secFree(global);
  secFree(user);
  if (u == NULL) {
    if (oidc_errno == OIDC_EJSONPARS) {
      printError("error in user configuration file: %s\n", oidc_serror());
      exit(oidc_errno);
    }
    return g;
  }
  if (g == NULL) {
    if (oidc_errno == OIDC_EJSONPARS) {
      printError("error in global configuration file: %s\n", oidc_serror());
      exit(oidc_errno);
    }
    return u;
  }
  cJSON* c = jsonMergePatch(g, u);
  secFreeJson(g);
  secFreeJson(u);
  if (c == NULL) {
    printError("error while merging global and user config\n");
    exit(EXIT_FAILURE);
  }
  return c;
}
