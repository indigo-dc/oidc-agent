#ifndef LIST_UTILS_H
#define LIST_UTILS_H

#include "../../lib/list/src/list.h"

char*   delimitedStringToJSONArray(char* str, char delimiter);
list_t* delimitedStringToList(char* str, char delimiter);
char*   listToDelimitedString(list_t* list, char delimiter);
list_t* intersectLists(list_t* a, list_t* b);
char*   listToJSONArray(list_t* list);

#endif  // LIST_UTILS_H
