#include <stdlib.h>
#include <string.h>
#include <syslog.h>

#include "oidc_array.h"


/** @fn void* arr_sort(void* arr, size_t numberElements, size_t elementSize, int (*comp_callback)(const void*, const void*))
 * @brief sorts array calling qsort
 * @param arr the array to be sorted
 * @param numberElements the number of elements in arr
 * @param elementSize the size of one element
 * @param comp_callback the comparison function used for sorting
 * @return the sorted array
 */
void* arr_sort(void* arr, size_t numberElements, size_t elementSize, int (*comp_callback)(const void*, const void*)) {
  qsort(arr, numberElements, elementSize, comp_callback);
  return arr;
}

/** @fn void* arr_find(void* arr, size_t numberElements, size_t elementSize, void* element, int (*comp_callback)(const void*, const void*)) 
 * @brief finds an element in an array.
 * @param arr the array that should be searched
 * @param numberElements the number of elements in arr
 * @param elementSize the size of one element
 * @param element the element to be found. 
 * @param comp_callback the comparison function
 * @return a pointer to the found element. If no element could be found
 * NULL is returned.
 */
void* arr_find(void* arr, size_t numberElements, size_t elementSize, void* element, int (*comp_callback)(const void*, const void*)) {
  arr_sort(arr, numberElements, elementSize, comp_callback);
  return bsearch(element, arr, numberElements, elementSize, comp_callback);
}

/** @fn void* arr_addElement(void* array, size_t* numberElements, size_t elementSize, void* element)  
 * @brief adds an element to an array 
 * @param array a pointer to the start of an array
 * @param numberElements a pointer to the number of elements in the array
 * @param elementSize the size of one element
 * @param element the element to be added. 
 * @return a pointer to the new array
 */
void* arr_addElement(void* array, size_t* numberElements, size_t elementSize, void* element) {
  void* tmp = realloc(array, elementSize * (*numberElements + 1));
  if (tmp==NULL) {
    syslog(LOG_AUTHPRIV|LOG_EMERG, "%s (%s:%d) realloc() failed: %m\n", __func__, __FILE__, __LINE__);
    free(array);
    return NULL;
  }
  array = tmp;
  memcpy(array + *numberElements, element, elementSize);
  (*numberElements)++;
  return array;
}

/** @fn void* arr_removeElement(void* array, size_t* numberElements, size_t elementSize, void* element, int (*comp_callback)(const void*, const void*))  
 * @brief removes an element from an array 
 * @param array a pointer to the start of an array
 * @param numberElements a pointer to the number of elements in the array
 * @param elementSize the size of one element
 * @param element the element to be removed. 
 * @param comp_callback the comparison function
 * @return a pointer to the new array
 */
void* arr_removeElement(void* array, size_t* numberElements, size_t elementSize, void* element, int (*comp_callback)(const void*, const void*)) {
  void* pos = arr_find(array, *numberElements, elementSize, element, comp_callback);
  if(NULL==pos)
    return array;
  memmove(pos, array + *numberElements - 1, elementSize);
  (*numberElements)--;
  void* tmp = realloc(array, elementSize * (*numberElements));
  if (tmp==NULL && *numberElements > 0) {
    syslog(LOG_AUTHPRIV|LOG_EMERG, "%s (%s:%d) realloc() failed: %m\n", __func__, __FILE__, __LINE__);
    free(array);
    return NULL;
  }
  array = tmp;
  return array;
}

