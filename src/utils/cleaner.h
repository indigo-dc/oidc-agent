#ifndef CLEANER_H
#define CLEANER_H

#include <stddef.h>

void* secAlloc(size_t size);
void* secRealloc(void* p, size_t size);
void  secFree(void* p);
void  secFreeN(void* p, size_t len);
// void  clearFreeString(char* s);
// void  clearFreeStringArray(char** arr, size_t size);

#endif  // CLEANER_H
