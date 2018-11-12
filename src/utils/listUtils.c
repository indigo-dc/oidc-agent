#include "listUtils.h"

#include "json.h"
#include "memory.h"
#include "oidc_error.h"
#include "stringUtils.h"

char* delimitedStringToJSONArray(char* str, char delimiter) {
  if (str == NULL) {
    oidc_setArgNullFuncError(__func__);
    return NULL;
  }

  size_t size  = strCountChar(str, delimiter) + 1;
  char*  copy  = oidc_sprintf("%s", str);
  char*  delim = oidc_sprintf("%c", delimiter);
  char*  json  = oidc_sprintf("\"%s\"", strtok(copy, delim));
  size_t i;
  for (i = 1; i < size; i++) {
    char* tmp = oidc_sprintf("%s, \"%s\"", json, strtok(NULL, delim));
    secFree(json);
    if (tmp == NULL) {
      secFree(delim);
      secFree(copy);
      return NULL;
    }
    json = tmp;
  }
  secFree(delim);
  secFree(copy);
  char* tmp = oidc_sprintf("[%s]", json);
  secFree(json);
  if (tmp == NULL) {
    return NULL;
  }
  return tmp;
}

char* listToJSONArrayString(list_t* list) {
  if (list == NULL) {
    oidc_setArgNullFuncError(__func__);
    return NULL;
  }

  cJSON* json = listToJSONArray(list);
  if (json == NULL) {
    return NULL;
  }
  char* str = jsonToString(json);
  secFreeJson(json);
  return str;
}

list_t* delimitedStringToList(char* str, char delimiter) {
  if (str == NULL) {
    oidc_setArgNullFuncError(__func__);
    return NULL;
  }

  char*   copy  = oidc_sprintf("%s", str);
  char*   delim = oidc_sprintf("%c", delimiter);
  list_t* list  = list_new();
  list->free    = (void (*)(void*)) & _secFree;
  list->match   = (int (*)(void*, void*)) & strequal;
  char* elem    = strtok(copy, delim);
  while (elem != NULL) {
    list_rpush(list, list_node_new(oidc_sprintf(elem)));
    elem = strtok(NULL, delim);
  }
  secFree(delim);
  secFree(copy);
  return list;
}

char* listToDelimitedString(list_t* list, char delimiter) {
  if (list == NULL) {
    return NULL;
  }
  list_node_t* node = list_at(list, 0);
  char*        str  = NULL;
  char*        tmp  = NULL;
  if (node == NULL) {
    str = oidc_sprintf("");
  } else {
    str = oidc_sprintf("%s", node->val);
  }
  unsigned int i;
  for (i = 1; i < list->len; i++) {
    tmp = oidc_sprintf("%s%c%s", str, delimiter, list_at(list, i)->val);
    secFree(str);
    if (tmp == NULL) {
      return NULL;
    }
    str = tmp;
  }
  return str;
}

// int delimitedStringToArray(const char* str, char delimiter, char** arr) {
//   size_t size = strCountChar(str, delimiter)+1;
//   if(arr==NULL) {
//     return size;
//   }
//   char* arr_str = oidc_sprintf("%s", str);
//   char* orig = arr_str;
//   int len = strlen(orig);
//   if(arr_str[0]=='[') { arr_str++;  }
//   if(arr_str[strlen(arr_str)-1]==']') { arr_str[strlen(arr_str)-1] = '\0'; }
//   char* delim = oidc_sprintf("%c", delimiter);
//   arr[0] = oidc_sprintf("%s", strtok(arr_str, delim));
//   unsigned int i;
//   for(i=1; i<size; i++) {
//     arr[i] = oidc_sprintf("%s", strtok(NULL, delim));
//   }
//   secFree(delim);
//   secFree(orig, len);
//   return i;
// }

list_t* intersectLists(list_t* a, list_t* b) {
  list_t* l = list_new();
  l->free   = (void (*)(void*)) & _secFree;
  l->match  = (int (*)(void*, void*)) & strequal;
  list_node_t*     node;
  list_iterator_t* it = list_iterator_new(a, LIST_HEAD);
  while ((node = list_iterator_next(it))) {
    list_node_t* n = findInList(b, node->val);
    if (n) {
      list_rpush(l, list_node_new(oidc_strcopy(n->val)));
    }
  }
  list_iterator_destroy(it);
  return l;
}

/**
 * a-b
 */
list_t* subtractLists(list_t* a, list_t* b) {
  list_t* l = list_new();
  l->free   = (void (*)(void*)) & _secFree;
  l->match  = (int (*)(void*, void*)) & strequal;
  list_node_t*     node;
  list_iterator_t* it = list_iterator_new(a, LIST_HEAD);
  while ((node = list_iterator_next(it))) {
    list_node_t* n = findInList(b, node->val);
    if (n == NULL) {
      list_rpush(l, list_node_new(oidc_strcopy(node->val)));
    }
  }
  list_iterator_destroy(it);
  return l;
}

list_node_t* findInList(list_t* l, void* v) {
  if (l == NULL) {
    return NULL;
  }
  return list_find(l, v);
}
