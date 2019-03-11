#ifndef OIDCD_INTERNAL_REQUEST_HANDLER_H
#define OIDCD_INTERNAL_REQUEST_HANDLER_H

#include "ipc/pipe.h"

void oidcd_handleUpdateRefreshToken(const struct ipcPipe, const char*,
                                    const char*);

#endif  // OIDCD_INTERNAL_REQUEST_HANDLER_H
