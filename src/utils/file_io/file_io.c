#define _XOPEN_SOURCE 700
#include "file_io.h"

#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "utils/listUtils.h"
#include "utils/logger.h"
#include "utils/memory.h"
#include "utils/string/stringUtils.h"

char* readFILE2(FILE* fp) {
  logger(DEBUG, "I'm reading a file step by step");
  size_t bsize   = 8;
  char*  buffer  = secAlloc(bsize + 1);
  size_t written = 0;
  while (1) {
    if (fread(buffer + written, bsize, 1, fp) != 1) {
      if (feof(fp)) {
        if (buffer[strlen(buffer) - 1] == '\n') {
          buffer[strlen(buffer) - 1] = '\0';
        }
        if (buffer[0] == '\0') {
          secFree(buffer);
          return NULL;
        }
        return buffer;
      }
      if (ferror(fp)) {
        oidc_setErrnoError();
        secFree(buffer);
        return NULL;
      }
    }
    written += bsize;
    buffer = secRealloc(buffer, written + bsize + 1);
  }
}

char* readFILE(FILE* fp) {
  if (fp == NULL) {
    oidc_setArgNullFuncError(__func__);
    return NULL;
  }

  if (fseek(fp, 0L, SEEK_END) != 0) {
    return readFILE2(fp);
  }
  long lSize = ftell(fp);
  rewind(fp);
  if (lSize < 0) {
    oidc_setErrnoError();
    logger(ERROR, "%s", oidc_serror());
    return NULL;
  }

  char* buffer = secAlloc(lSize + 1);
  if (!buffer) {
    logger(ERROR, "memory alloc failed in function %s for %ld bytes", __func__,
           lSize);
    oidc_errno = OIDC_EALLOC;
    return NULL;
  }

  if (1 != fread(buffer, lSize, 1, fp)) {
    if (feof(fp)) {
      oidc_errno = OIDC_EEOF;
    } else {
      oidc_errno = OIDC_EFREAD;
    }
    secFree(buffer);
    logger(ERROR, "entire read failed in function %s", __func__);
    return NULL;
  }
  return buffer;
}

/** @fn char* readFile(const char* path)
 * @brief reads a file and returns a pointer to the content
 * @param path the file to be read
 * @return a pointer to the file content. Has to be freed after usage. On
 * failure NULL is returned and oidc_errno is set.
 */
char* readFile(const char* path) {
  logger(DEBUG, "Reading file: %s", path);

  FILE* fp = fopen(path, "rb");
  if (!fp) {
    logger(NOTICE, "%m\n");
    oidc_errno = OIDC_EFOPEN;
    return NULL;
  }

  char* ret = readFILE(fp);
  fclose(fp);
  return ret;
}

char* getLineFromFILE(FILE* fp) {
  char*  buf = NULL;
  size_t len = 0;
  int    n;
  if ((n = getline(&buf, &len, fp)) < 0) {
    logger(NOTICE, "getline: %s", strerror(errno));
    oidc_errno = OIDC_EIN;
    return NULL;
  }
  if (buf[n - 1] == '\n') {
    buf[n - 1] = 0;  // removing '\n' at the end
  }
  char* secFreeAblePointer =
      oidc_strcopy(buf);  // Because getline allocates memory using malloc and
                          // not secAlloc, we cannot free buf with secFree. To
                          // be able to do so we copy the buf to memory
                          // allocated with secAlloc and free buf using secFreeN
  secFreeN(buf, n);
  return secFreeAblePointer;
}

char* getLineFromFile(const char* path) {
  FILE* fp = fopen(path, "rb");
  if (!fp) {
    logger(NOTICE, "%m\n");
    oidc_errno = OIDC_EFOPEN;
    return NULL;
  }

  char* ret = getLineFromFILE(fp);
  fclose(fp);
  return ret;
}

/** @fn void writeFile(const char* path, const char* text)
 * @brief writes text to a file
 * @note \p text has to be nullterminated and must not contain nullbytes.
 * @param path the file to be written
 * @param text the nullterminated text to be written
 * @return OIDC_OK on success, OID_EFILE if an error occurred. The system sets
 * errno.
 */
oidc_error_t writeFile(const char* path, const char* text) {
  if (path == NULL || text == NULL) {
    oidc_setArgNullFuncError(__func__);
    return oidc_errno;
  }
  FILE* f = fopen(path, "w");
  if (f == NULL) {
    logger(ALERT, "Error opening file '%s' in function writeToFile().\n", path);
    return OIDC_EFOPEN;
  }
  fprintf(f, "%s", text);
  fclose(f);
  return OIDC_SUCCESS;
}

oidc_error_t appendFile(const char* path, const char* text) {
  if (path == NULL || text == NULL) {
    oidc_setArgNullFuncError(__func__);
    return oidc_errno;
  }
  FILE* f = fopen(path, "a");
  if (f == NULL) {
#ifndef __APPLE__  // logger on MAC uses this function so don't use logger if
                   // something goes wrong
    logger(ALERT, "Error opening file '%s' in function appendFile().\n", path);
#endif
    return OIDC_EFOPEN;
  }
  fprintf(f, "%s\n", text);
  fclose(f);
  return OIDC_SUCCESS;
}

/** @fn int fileDoesExist(const char* path)
 * @brief checks if a file exists
 * @param path the path to the file to be checked
 * @return 1 if the file does exist, 0 if not
 */
int fileDoesExist(const char* path) {
  return path ? access(path, F_OK) == 0 ? 1 : 0 : 0;
}

/** @fn int dirExists(const char* path)
 * @brief checks if a directory exists
 * @param path the path to the directory to be checked
 * @return @c OIDC_DIREXIST_OK if the directory does exist, @c OIDC_DIREXIST_NO
 * if not, @c OIDC_DIREXIST_ERROR if the directory if an error occurred
 */
int dirExists(const char* path) {
  DIR* dir = opendir(path);
  if (dir) { /* Directory exists. */
    closedir(dir);
    return OIDC_DIREXIST_OK;
  } else if (ENOENT == errno) { /* Directory does not exist. */
    return OIDC_DIREXIST_NO;
  } else if (EACCES == errno) {
    logger(NOTICE, "opendir: %m");
    oidc_setErrnoError();
    return OIDC_DIREXIST_NO;
  } else { /* opendir() failed for some other reason. */
    logger(ALERT, "opendir: %m");
    oidc_setErrnoError();
    return OIDC_DIREXIST_ERROR;
  }
}

oidc_error_t createDir(const char* path) {
  if (mkdir(path, 0777) != 0) {
    oidc_setErrnoError();
    return oidc_errno;
  }
  return OIDC_SUCCESS;
}

/** @fn int removeFile(const char* path)
 * @brief removes a file
 * @param path the path to the file to be removed
 * @return On success, 0 is returned.  On error, -1 is returned, and errno is
 * set appropriately.
 */
int removeFile(const char* path) { return unlink(path); }

list_t* _getLinesFromFile(const char* path, const unsigned char ignoreComments,
                          const char commentChar) {
  if (path == NULL) {
    oidc_setArgNullFuncError(__func__);
    return NULL;
  }
  logger(DEBUG, "Getting Lines from file: %s", path);
  FILE* fp = fopen(path, "r");
  if (fp == NULL) {
    oidc_setErrnoError();
    return NULL;
  }

  list_t* lines = list_new();
  lines->free   = _secFree;
  lines->match  = (matchFunction)strequal;

  char*   line = NULL;
  size_t  len  = 0;
  ssize_t read = 0;
  while ((read = getline(&line, &len, fp)) != -1) {
    if (line[strlen(line) - 1] == '\n') {
      line[strlen(line) - 1] = '\0';
    }
    if (!ignoreComments || commentChar != firstNonWhiteSpaceChar(line)) {
      list_rpush(lines, list_node_new(oidc_strcopy(line)));
    }
    secFreeN(line, len);
  }
  secFreeN(line, len);
  fclose(fp);
  return lines;
}

list_t* getLinesFromFile(const char* path) {
  return _getLinesFromFile(path, 0, 0);
}

list_t* getLinesFromFileWithoutComments(const char* path) {
  return _getLinesFromFile(path, 1, DEFAULT_COMMENT_CHAR);
}

oidc_error_t mkpath(const char* p, const mode_t mode) {
  if (p == NULL) {
    return OIDC_SUCCESS;
  }
  char* path = oidc_strcopy(p);
  if (lastChar(path) == '/') {
    lastChar(path) = '\0';
  }
  char* pos = path;
  while ((pos = strchr(pos + 1, '/')) != NULL) {
    *pos = '\0';
    if (mkdir(path, mode) && errno != EEXIST) {
      secFree(path);
      oidc_setErrnoError();
      return oidc_errno;
    }
    *pos = '/';
  }
  if (mkdir(path, mode) && errno != EEXIST) {
    secFree(path);
    oidc_setErrnoError();
    return oidc_errno;
  }
  secFree(path);
  return OIDC_SUCCESS;
}
