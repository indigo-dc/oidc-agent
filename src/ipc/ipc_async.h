#ifndef IPC_ASYNC_H
#define IPC_ASYNC_H

#include "connection.h"

#include "../../lib/list/src/list.h"

struct connection* ipc_async(struct connection listencon, list_t* connections);

#endif  // IPC_ASYNC_H
