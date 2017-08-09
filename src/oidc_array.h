#ifndef OIDC_ARRAY_H
#define OIDC_ARRAY_H

#include <stddef.h>

void* arr_addElement(void* array, size_t* numberElements, size_t elementSize, void* element) ;
void* arr_removeElement(void* array, size_t* numberElements, size_t elementSize, void* element, int (*comp_callback)(const void*, const void*)) ;
void* arr_find(void* arr, size_t numberElements, size_t elementSize, void* element, int (*comp_callback)(const void*, const void*)) ;
void* arr_sort(void* arr, size_t numberElements, size_t elementSize, int (*comp_callback)(const void*, const void*)) ;


#endif // OIDC_ARRAY_H
