#include <stdlib.h>
#include <string.h>
#include <syslog.h>

#include "oidc_array.h"



void* arr_sort(void* arr, size_t numberElements, size_t elementSize, int (*comp_callback)(const void*, const void*)) {
  qsort(arr, numberElements, elementSize, comp_callback);
  return arr;
}

void* arr_find(void* arr, size_t numberElements, size_t elementSize, void* element, int (*comp_callback)(const void*, const void*)) {
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "arr p %p",arr);
  arr_sort(arr, numberElements, elementSize, comp_callback);
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "arr p %p",arr);
  return bsearch(element, arr, numberElements, elementSize, comp_callback);
}

void* arr_addElement(void* array, size_t* numberElements, size_t elementSize, void* element) {
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "add array %p", array);
    array = realloc(array, elementSize * (*numberElements + 1));
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "add array %p", array);
    memcpy(array + *numberElements, element, elementSize);
    (*numberElements)++;
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "add array %p", array);
    return array;
}

void* arr_removeElement(void* array, size_t* numberElements, size_t elementSize, void* element, int (*comp_callback)(const void*, const void*)) {
    void* pos = arr_find(array, *numberElements, elementSize, element, comp_callback);
    if(NULL==pos)
      return NULL;
    memmove(pos, array + *numberElements - 1, elementSize);
    (*numberElements)--;
    array = realloc(array, elementSize * (*numberElements));
    return array;
}

