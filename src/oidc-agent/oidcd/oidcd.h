#ifndef OIDC_DAEMON_H
#define OIDC_DAEMON_H

#include "ipc/pipe.h"
#include "oidc-agent/oidc-agent_options.h"

int oidcd_main(struct ipcPipe, const struct arguments*);

#endif  // OIDC_DAEMON_H
