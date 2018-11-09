#ifndef ADD_HANDLER_H
#define ADD_HANDLER_H

#include <time.h>

struct lifetimeArg {
  time_t lifetime;
  short  argProvided;
};

char* getAccountConfig(char* account);
void  add_handleAdd(char* account, struct lifetimeArg lifetime);
void  add_handleRemove(const char* account);
void  add_handleRemoveAll();
void  add_handleList();
void  add_handlePrint(char* account);
void  add_handleLock(int lock);
void  add_assertAgent();

#endif  // ADD_HANDLER_H
