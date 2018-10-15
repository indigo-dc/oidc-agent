#ifndef ADD_HANDLER_H
#define ADD_HANDLER_H

#include <time.h>

struct lifetimeArg {
  time_t lifetime;
  short  argProvided;
};

char* getAccountConfig(char* account);
void  add_handleAddAndRemove(char* account, int remove,
                             struct lifetimeArg lifetime);
void  add_handleList();
void  add_handlePrint(char* account);

#endif  // ADD_HANDLER_H
