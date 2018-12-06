#ifndef LIST_UTILS_H
#define LIST_UTILS_H

#include "list/list.h"

char*        delimitedStringToJSONArray(char* str, char delimiter);
list_t*      delimitedStringToList(const char* str, char delimiter);
char*        listToDelimitedString(list_t* list, char delimiter);
list_t*      intersectLists(list_t* a, list_t* b);
list_t*      subtractLists(list_t* a, list_t* b);
char*        listToJSONArrayString(list_t* list);
list_node_t* findInList(list_t* l, void* v);
void         list_removeIfFound(list_t* l, void* v);
void         list_mergeSort(list_t* l, int (*comp)(const void*, const void*));

#endif  // LIST_UTILS_H
