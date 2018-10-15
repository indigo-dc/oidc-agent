#ifndef IPC_ASYNC_H
#define IPC_ASYNC_H

#include "connection.h"

#include "../../lib/list/src/list.h"

#include <time.h>

struct connection* ipc_async(struct connection listencon, list_t* connections,
                             time_t death);

#endif  // IPC_ASYNC_H
