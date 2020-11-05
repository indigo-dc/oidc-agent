#ifndef LIST_UTILS_H
#define LIST_UTILS_H

#include "wrapper/list.h"

#define LIST_CREATE_COPY_VALUES 1
#define LIST_CREATE_DONT_COPY_VALUES 0

typedef int (*matchFunction)(const void*, const void*);
typedef void (*freeFunction)(void*);

char*        delimitedStringToJSONArray(char* str, char delimiter);
list_t*      delimitedStringToList(const char* str, char delimiter);
char*        listToDelimitedString(list_t* list, char* delimiter);
list_t*      intersectLists(list_t* a, list_t* b);
list_t*      subtractLists(list_t* a, list_t* b);
char*        subtractListStrings(const char* a, const char* b, const char del);
char*        listToJSONArrayString(list_t* list);
list_node_t* findInList(list_t* l, const void* v);
list_t*      findAllInList(list_t* l, const void* v);
void         list_removeIfFound(list_t* l, const void* v);
void         list_mergeSort(list_t* l, int (*comp)(const void*, const void*));
void         secFreeList(list_t* l);
list_t*      createList(int copyValues, char* s, ...);
list_t*      list_addStringIfNotFound(list_t* l, char* v);
int          listValid(const list_t* l);

#endif  // LIST_UTILS_H
