#ifndef ADD_HANDLER_H
#define ADD_HANDLER_H

#include "oidc-add/oidc-add_options.h"

void add_handleAdd(char* account, struct arguments* arguments);
void add_handleRemove(const char* account);
void add_handleRemoveAll();
void add_handlePrint(char* account, struct arguments* arguments);
void add_handleLock(int lock);
void add_handleListLoadedAccounts();

#endif  // ADD_HANDLER_H
