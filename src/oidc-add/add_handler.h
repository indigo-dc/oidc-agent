#ifndef ADD_HANDLER_H
#define ADD_HANDLER_H

#include "oidc-add/oidc-add_options.h"

void add_handleAdd(const char* account, struct arguments* arguments);
void add_handleRemove(const char* account, struct arguments* arguments);
void add_handleRemoveAll(struct arguments* arguments);
void add_handlePrint(const char* account, struct arguments* arguments);
void add_handleLock(int lock, struct arguments* arguments);
void add_handleListLoadedAccounts(struct arguments* arguments);

#endif  // ADD_HANDLER_H
