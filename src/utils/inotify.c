#include "inotify.h"

#ifdef __linux__

#include "memory.h"
#include "oidc_error.h"
#include "string/stringUtils.h"

#define _XOPEN_SOURCE 500
#include <libgen.h>
#include <sys/inotify.h>
#include <unistd.h>

#define EVENT_SIZE (sizeof(struct inotify_event))
#define EVENT_BUF_LEN (1024 * (EVENT_SIZE + 16))

int inotify_watch(const char* file_path) {
  if (file_path == NULL) {
    oidc_setArgNullFuncError(__func__);
    return -1;
  }

  char* fp_copy   = oidc_strcopy(file_path);
  char* directory = oidc_strcopy(dirname(fp_copy));  // Get directory part
  secFree(fp_copy);
  fp_copy        = oidc_strcopy(file_path);
  char* filename = oidc_strcopy(basename(fp_copy));  // Get filename part
  secFree(fp_copy);

  // Create inotify instance
  int fd = inotify_init();
  if (fd < 0) {
    oidc_setErrnoError();
    secFree(directory);
    secFree(filename);
    return -1;
  }

  // Add a watch for the directory
  int wd = inotify_add_watch(fd, directory, IN_MOVED_FROM | IN_MOVED_TO);
  if (wd == -1) {
    oidc_setErrnoError();
    close(fd);
    secFree(directory);
    secFree(filename);
    return -1;
  }

  secFree(directory);
  secFree(filename);
  return fd;
}

oidc_error_t inotify_read(int fd, const char* filename, void (*callback)()) {
  char buffer[EVENT_BUF_LEN];

  ssize_t length = read(fd, buffer, EVENT_BUF_LEN);
  if (length < 0) {
    oidc_setErrnoError();
    return oidc_errno;
  }

  size_t i = 0;
  while (i < (size_t)length) {
    struct inotify_event* event = (struct inotify_event*)&buffer[i];

    if ((event->mask & IN_MOVED_FROM || event->mask & IN_MOVED_TO) &&
        strequal(event->name, filename)) {
      callback();
      return OIDC_SUCCESS;
    }
    i += EVENT_SIZE + event->len;
  }
  return OIDC_SUCCESS;
}

#endif  //__linux__