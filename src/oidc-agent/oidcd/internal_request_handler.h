#ifndef OIDCD_INTERNAL_REQUEST_HANDLER_H
#define OIDCD_INTERNAL_REQUEST_HANDLER_H

#include "ipc/pipe.h"

void oidcd_handleUpdateRefreshToken(struct ipcPipe, const char*, const char*);
void oidcd_handleUpdateIssuer(struct ipcPipe pipes, const char* issuer_url,
                              const char* short_name, const char* action);

#endif  // OIDCD_INTERNAL_REQUEST_HANDLER_H
