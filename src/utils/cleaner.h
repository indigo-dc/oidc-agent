#ifndef CLEANER_H
#define CLEANER_H

#include <stddef.h>

void clearFree(void* p, size_t len);
void clearFreeString(char* s);
void clearFreeStringArray(char** arr, size_t size);

#endif  // CLEANER_H
