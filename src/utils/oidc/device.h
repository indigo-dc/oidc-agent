#ifndef OIDC_AGENT_DEVICE_H
#define OIDC_AGENT_DEVICE_H

#include <stddef.h>
#include <time.h>

#include "ipc/pipe.h"

char* pollDeviceCode(const char* json_device, size_t interval,
                     time_t expires_at, unsigned char remote,
                     unsigned char only_at);
char* agent_pollDeviceCode(const char* json_device, size_t interval,
                           time_t expires_at, unsigned char only_at,
                           struct ipcPipe* pipes);

#endif  // OIDC_AGENT_DEVICE_H
