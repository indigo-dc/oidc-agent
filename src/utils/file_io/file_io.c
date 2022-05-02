#define _XOPEN_SOURCE 700
#include "file_io.h"

#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "defines/msys.h"
#include "utils/listUtils.h"
#include "utils/logger.h"
#include "utils/memory.h"
#include "utils/string/stringUtils.h"

oidc_error_t readBinaryFILE2(FILE* fp, char** buffer, size_t* size) {
  logger(DEBUG, "I'm reading a file step by step");
  const size_t bsize = 8;
  *buffer            = secAlloc(bsize + 1);
  *size              = 0;
  while (1) {
    size_t written = fread(*buffer + *size, 1, bsize, fp);
    *size += written;
    if (written != bsize) {
      if (feof(fp)) {
        if ((*buffer)[*size - 1] == '\n') {
          (*buffer)[*size - 1] = '\0';
        }
        if ((*buffer)[0] == '\0') {
          secFree(*buffer);
        }
        return OIDC_SUCCESS;
      }
      if (ferror(fp)) {
        oidc_setErrnoError();
        secFree(*buffer);
        return oidc_errno;
      }
    }
    *buffer = secRealloc(*buffer, *size + bsize + 1);
  }
}

oidc_error_t readBinaryFILE(FILE* fp, char** buffer, size_t* size) {
  if (fp == NULL) {
    oidc_setArgNullFuncError(__func__);
    return oidc_errno;
  }

  if (fseek(fp, 0L, SEEK_END) != 0) {
    return readBinaryFILE2(fp, buffer, size);
  }
  long lSize = ftell(fp);
  rewind(fp);
  if (lSize < 0) {
    oidc_setErrnoError();
    logger(ERROR, "%s", oidc_serror());
    return oidc_errno;
  }

  *buffer = secAlloc(lSize + 1);
  if (*buffer == NULL) {
    logger(ERROR, "memory alloc failed in function %s for %ld bytes", __func__,
           lSize);
    oidc_errno = OIDC_EALLOC;
    return oidc_errno;
  }

  if (1 != fread(*buffer, lSize, 1, fp)) {
    if (feof(fp)) {
      oidc_errno = OIDC_EEOF;
    } else {
      oidc_errno = OIDC_EFREAD;
    }
    secFree(*buffer);
    logger(ERROR, "entire read failed in function %s", __func__);
    return oidc_errno;
  }
  *size = lSize;
  return OIDC_SUCCESS;
}
char* readFILE(FILE* fp) {
  char*  buffer = NULL;
  size_t size;
  if (readBinaryFILE(fp, &buffer, &size) != OIDC_SUCCESS) {
    secFree(buffer);
    return NULL;
  }
  return buffer;
}

oidc_error_t readBinaryFile(const char* path, char** buffer, size_t* size) {
  logger(DEBUG, "Reading file: %s", path);

  FILE* fp = fopen(path, "rb");
  if (!fp) {
    logger(NOTICE, "%m\n");
    oidc_errno = OIDC_EFOPEN;
    return oidc_errno;
  }

  oidc_error_t ret = readBinaryFILE(fp, buffer, size);
  fclose(fp);
  return ret;
}

/** @fn char* readFile(const char* path)
 * @brief reads a file and returns a pointer to the content
 * @param path the file to be read
 * @return a pointer to the file content. Has to be freed after usage. On
 * failure NULL is returned and oidc_errno is set.
 */
char* readFile(const char* path) {
  char*  buffer = NULL;
  size_t size;
  if (readBinaryFile(path, &buffer, &size) != OIDC_SUCCESS) {
    secFree(buffer);
    return NULL;
  }
  return buffer;
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
#ifdef __linux__  // logger on MAC uses this function so don't use logger if
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

int _mkdir(const char* path, int mode) {
#ifdef MINGW
  return mkdir(path);
#else
  return mkdir(path, mode);
#endif
}

oidc_error_t createDir(const char* path) {
  if (_mkdir(path, 0777) != 0) {
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

#ifdef MINGW
int getline(char** lineptr, size_t* n, FILE* stream) {
  static char  line[256];
  char*        ptr;
  unsigned int len;

  if (lineptr == NULL || n == NULL) {
    errno = EINVAL;
    return -1;
  }

  if (ferror(stream)) {
    return -1;
  }

  if (feof(stream)) {
    return -1;
  }

  fgets(line, 256, stream);

  ptr = strchr(line, '\n');
  if (ptr) {
    *ptr = '\0';
  }

  len = strlen(line);

  if ((len + 1) < 256) {
    ptr = realloc(*lineptr, 256);
    if (ptr == NULL)
      return (-1);
    *lineptr = ptr;
    *n       = 256;
  }

  strcpy(*lineptr, line);
  return (len);
}
#endif

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
    if (_mkdir(path, mode) && errno != EEXIST) {
      secFree(path);
      oidc_setErrnoError();
      return oidc_errno;
    }
    *pos = '/';
  }
  if (_mkdir(path, mode) && errno != EEXIST) {
    secFree(path);
    oidc_setErrnoError();
    return oidc_errno;
  }
  secFree(path);
  return OIDC_SUCCESS;
}
