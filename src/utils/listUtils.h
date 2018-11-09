#ifndef LIST_UTILS_H
#define LIST_UTILS_H

#include "list/list.h"

char*   delimitedStringToJSONArray(char* str, char delimiter);
list_t* delimitedStringToList(char* str, char delimiter);
char*   listToDelimitedString(list_t* list, char delimiter);
list_t* intersectLists(list_t* a, list_t* b);
list_t* subtractLists(list_t* a, list_t* b);
char*   listToJSONArrayString(list_t* list);

#endif  // LIST_UTILS_H
