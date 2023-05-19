#include "oidc_file_io.h"

#include <fcntl.h>
#include <libgen.h>
#include <stdlib.h>
#include <unistd.h>

#include "defines/settings.h"
#include "file_io.h"
#include "utils/file_io/fileUtils.h"
#include "utils/listUtils.h"
#include "utils/logger.h"
#include "utils/memory.h"
#include "utils/string/stringUtils.h"

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

list_t* getPossibleOidcDirLocations() {
  char* pathDotConfig = fillEnvVarsInPath(AGENTDIR_LOCATION_CONFIG);
  char* pathDot       = fillEnvVarsInPath(AGENTDIR_LOCATION_DOT);
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
  char*   ret               = getExistingLocation(possibleLocations);
  secFreeList(possibleLocations);
  return ret;
}

oidc_error_t createOidcDir() {
#ifdef __MSYS__
  char* path = fillEnvVarsInPath(AGENTDIR_LOCATION_CONFIG);
#else
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
        secFreeList(possibleLocations);
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
  char* tmp = oidc_strcopy(path);
  path      = tmp;
  secFreeList(possibleLocations);
#endif
  if (path == NULL) {
    return oidc_errno;
  }
  logger(DEBUG, "Using '%s' as oidcdir.", path);
  switch (dirExists(path)) {
    case OIDC_DIREXIST_ERROR: secFree(path); return oidc_errno;
    case OIDC_DIREXIST_OK: secFree(path); return OIDC_SUCCESS;
  }
  logger(DEBUG, "Creating '%s' as oidcdir.", path);
#ifdef __MSYS__
  oidc_error_t ret = mkpath(path, 0777);
#else
  oidc_error_t ret = createDir(path);
#endif
  secFree(path);
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
  char* path     = oidc_pathcat(oidc_dir, filename);
  secFree(oidc_dir);
  return path;
}

list_t* getLinesFromOidcFile(const char* filename) {
  char*   path = concatToOidcDir(filename);
  list_t* ret  = getLinesFromFile(path);
  secFree(path);
  return ret;
}

list_t* getLinesFromOidcFileWithoutComments(const char* filename) {
  char*   path = concatToOidcDir(filename);
  list_t* ret  = getLinesFromFileWithoutComments(path);
  secFree(path);
  return ret;
}

char* getFileContentFromOidcFileAfterLine(const char*   filename,
                                          const char*   lineContentPrefix,
                                          long          startChar,
                                          unsigned char allIfLineNotFound) {
  char* path = concatToOidcDir(filename);
  char* ret  = getFileContentAfterLine(path, lineContentPrefix, startChar,
                                       allIfLineNotFound);
  secFree(path);
  return ret;
}
