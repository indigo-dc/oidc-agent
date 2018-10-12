#ifndef MEMORY_H
#define MEMORY_H

#include <stddef.h>

void* secAlloc(size_t size);
void* secRealloc(void* p, size_t size);
void  secFree(void* p);
void  secFreeN(void* p, size_t len);
void  secFreeArray(char** arr, size_t size);

#endif  // MEMORY_H
