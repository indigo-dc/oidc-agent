#include "oidc_file_io.h"
#include "defines/settings.h"
#include "file_io.h"
#include "utils/listUtils.h"
#include "utils/logger.h"
#include "utils/memory.h"
#include "utils/printer.h"
#include "utils/stringUtils.h"

#include <fcntl.h>
#include <libgen.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/** @fn char* readOidcFile(const char* filename)
 * @brief reads a file located in the oidc dir and returns a pointer to the
 * content
 * @param filename the filename of the file
 * @return a pointer to the file content. Has to be freed after usage.
 */
char* readOidcFile(const char* filename) {
  char* path = concatToOidcDir(filename);
  char* c    = readFile(path);
  secFree(path);
  return c;
}

/** @fn void writeOidcFile(const char* filename, const char* text)
 * @brief writes text to a file located in the oidc directory
 * @note \p text has to be nullterminated and must not contain nullbytes.
 * @param filename the file to be written
 * @param text the nullterminated text to be written
 * @return OIDC_OK on success, OID_EFILE if an error occurred. The system sets
 * errno.
 */
oidc_error_t writeOidcFile(const char* filename, const char* text) {
  char*        path = concatToOidcDir(filename);
  oidc_error_t er   = writeFile(path, text);
  secFree(path);
  return er;
}

oidc_error_t appendOidcFile(const char* filename, const char* text) {
  char*        path = concatToOidcDir(filename);
  oidc_error_t er   = appendFile(path, text);
  secFree(path);
  return er;
}

/** @fn int oidcFileDoesExist(const char* filename)
 * @brief checks if a file exists in the oidc dir
 * @param filename the file to be checked
 * @return 1 if the file does exist, 0 if not
 */
int oidcFileDoesExist(const char* filename) {
  char* path = concatToOidcDir(filename);
  int   b    = fileDoesExist(path);
  secFree(path);
  return b;
}

char* getNonTildePath(const char* path_in) {
  if (path_in == NULL) {
    return NULL;
  }
  if (path_in[0] == '~') {
    char* home = getenv("HOME");
    if (home == NULL) {
      oidc_errno = OIDC_EERROR;
      oidc_seterror("Environment variable HOME is not set, cannot resolve ~.");
      return NULL;
    }
    if (strlen(path_in) == 1) {
      return oidc_strcopy(home);
    }
    return oidc_strcat(home, path_in + 1);
  } else {
    return oidc_strcopy(path_in);
  }
}

list_t* getPossibleOidcDirLocations() {
  char* pathDotConfig = getNonTildePath(AGENTDIR_LOCATION_CONFIG);
  char* pathDot       = getNonTildePath(AGENTDIR_LOCATION_DOT);
  if (pathDotConfig == NULL && pathDot == NULL) {
    return NULL;
  }
  list_t* possibleLocations = createList(0, pathDotConfig, pathDot, NULL);
  possibleLocations->free   = _secFree;
  char* locFromEnv          = getenv(OIDC_CONFIG_DIR_ENV_NAME);
  if (locFromEnv) {
    list_lpush(possibleLocations, list_node_new(oidc_strcopy(locFromEnv)));
  }
  return possibleLocations;
}

/** @fn char* getOidcDir()
 * @brief get the oidc directory path
 * @return a pointer to the oidc directory path. Has to be freed after usage. If
 * no oidc dir is found, NULL is returned
 */
char* getOidcDir() {
  list_t* possibleLocations = getPossibleOidcDirLocations();
  if (possibleLocations == NULL) {
    return NULL;
  }
  list_node_t*     node;
  list_iterator_t* it = list_iterator_new(possibleLocations, LIST_HEAD);
  while ((node = list_iterator_next(it))) {
    char* path = node->val;
    switch (dirExists(path)) {
      case OIDC_DIREXIST_ERROR:
        list_iterator_destroy(it);
        list_destroy(possibleLocations);
        return NULL;
      case OIDC_DIREXIST_OK:
        list_iterator_destroy(it);
        char* ret = withTrailingSlash(path);
        list_destroy(possibleLocations);
        return ret;
    }
  }
  list_iterator_destroy(it);
  list_destroy(possibleLocations);
  return NULL;
}

oidc_error_t createOidcDir() {
  list_t* possibleLocations = getPossibleOidcDirLocations();
  if (possibleLocations == NULL) {
    return oidc_errno;
  }
  char*            path = NULL;
  list_node_t*     node;
  list_iterator_t* it = list_iterator_new(possibleLocations, LIST_HEAD);
  unsigned char    foundParent = 0;
  while ((node = list_iterator_next(it))) {
    path         = node->val;
    char* tmp    = oidc_strcopy(path);
    char* parent = dirname(tmp);
    switch (dirExists(parent)) {
      case OIDC_DIREXIST_ERROR:
        secFree(tmp);
        list_iterator_destroy(it);
        list_destroy(possibleLocations);
        return oidc_errno;
      case OIDC_DIREXIST_OK:
        foundParent = 1;
        secFree(tmp);
        break;
    }
    if (foundParent) {
      break;
    }
    secFree(tmp);
    path = NULL;
  }
  list_iterator_destroy(it);
  if (path == NULL) {
    list_destroy(possibleLocations);
    return oidc_errno;
  }
  logger(DEBUG, "Using '%s' as oidcdir.", path);
  switch (dirExists(path)) {
    case OIDC_DIREXIST_ERROR:
      list_destroy(possibleLocations);
      return oidc_errno;
    case OIDC_DIREXIST_OK: list_destroy(possibleLocations); return OIDC_SUCCESS;
  }
  logger(DEBUG, "Creating '%s' as oidcdir.", path);
  oidc_error_t ret        = createDir(path);
  char* issuerconfig_path = oidc_sprintf("%s/%s", path, ISSUER_CONFIG_FILENAME);
  list_destroy(possibleLocations);
  int fd = open(issuerconfig_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
  close(fd);
  secFree(issuerconfig_path);
  return ret;
}

/** @fn int removeOidcFile(const char* filename)
 * @brief removes a file located in the oidc dir
 * @param filename the filename of the file to be removed
 * @return On success, 0 is returned.  On error, -1 is returned, and errno is
 * set appropriately.
 */
int removeOidcFile(const char* filename) {
  char* path = concatToOidcDir(filename);
  int   r    = removeFile(path);
  secFree(path);
  return r;
}

char* concatToOidcDir(const char* filename) {
  char* oidc_dir = getOidcDir();
  char* path     = oidc_strcat(oidc_dir, filename);
  secFree(oidc_dir);
  return path;
}

list_t* getLinesFromOidcFile(const char* filename) {
  char*   path = concatToOidcDir(filename);
  list_t* ret  = getLinesFromFile(path);
  secFree(path);
  return ret;
}

/**
 * @brief updates the issuer.config file.
 * If the issuer url is not already in the issuer.config file, it will be added.
 * @param issuer_url the issuer url to be added
 * @param shortname will be used as the default account config for this issuer
 */
void updateIssuerConfig(const char* issuer_url, const char* shortname) {
  if (issuer_url == NULL || shortname == NULL) {
    return;
  }
  char* issuers = readOidcFile(ISSUER_CONFIG_FILENAME);
  char* new_issuers;
  if (issuers) {
    if (strSubStringCase(issuers, issuer_url)) {
      secFree(issuers);
      return;
    }
    new_issuers = oidc_sprintf("%s\n%s %s", issuers, issuer_url, shortname);
    secFree(issuers);
  } else {
    new_issuers = oidc_sprintf("%s %s", issuer_url, shortname);
  }
  if (new_issuers == NULL) {
    logger(ERROR, "%s", oidc_serror());
  } else {
    writeOidcFile(ISSUER_CONFIG_FILENAME, new_issuers);
    secFree(new_issuers);
  }
}
