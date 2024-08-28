#ifndef OIDC_AGENT_INOTIFY_H
#define OIDC_AGENT_INOTIFY_H

#ifdef __linux__

#include "oidc_error.h"

oidc_error_t inotify_read(int fd, const char* filename, void (*callback)());
int          inotify_watch(const char* file_path);

#endif  // __linux__
#endif  // OIDC_AGENT_INOTIFY_H
