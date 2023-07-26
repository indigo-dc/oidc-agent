#include "listUtils.h"

#include <stdarg.h>
#include <string.h>

#include "json.h"
#include "memory.h"
#include "utils/oidc_error.h"
#include "utils/printer.h"
#include "utils/string/stringUtils.h"

char* delimitedStringToJSONArray(const char* str, char delimiter) {
  return delimitedStringToJSONArrayFmt(str, delimiter, "\"%s\"");
}

char* delimitedStringToJSONArrayFmt(const char* str, char delimiter,
                                    const char* valueFmt) {
  if (str == NULL) {
    oidc_setArgNullFuncError(__func__);
    return NULL;
  }

  size_t size  = strCountChar(str, delimiter) + 1;
  char*  copy  = oidc_sprintf("%s", str);
  char*  delim = oidc_sprintf("%c", delimiter);
  char*  json  = oidc_sprintf(valueFmt, strtok(copy, delim));
  size_t i;
  char*  fmt = oidc_strcat("%s, ", valueFmt);
  for (i = 1; i < size; i++) {
    char* tmp = oidc_sprintf(fmt, json, strtok(NULL, delim));
    secFree(json);
    if (tmp == NULL) {
      secFree(delim);
      secFree(copy);
      secFree(fmt);
      return NULL;
    }
    json = tmp;
  }
  secFree(fmt);
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

  cJSON* json = stringListToJSONArray(list);
  if (json == NULL) {
    return NULL;
  }
  char* str = jsonToString(json);
  secFreeJson(json);
  return str;
}

list_t* newListWithSingleValue(const char* str) {
  list_t* list = list_new();
  list->free   = (void (*)(void*)) & _secFree;
  list->match  = (matchFunction)strequal;
  list_rpush(list, list_node_new(oidc_strcopy(str)));
  return list;
}

list_t* delimitedStringToList(const char* str, char delimiter) {
  if (str == NULL) {
    oidc_setArgNullFuncError(__func__);
    return NULL;
  }

  char*   copy  = oidc_strcopy(str);
  char*   delim = oidc_sprintf("%c", delimiter);
  list_t* list  = list_new();
  list->free    = (void (*)(void*)) & _secFree;
  list->match   = (matchFunction)strequal;
  char* elem    = strtok(copy, delim);
  while (elem != NULL) {
    list_rpush(list, list_node_new(oidc_strcopy(elem)));
    elem = strtok(NULL, delim);
  }
  secFree(delim);
  secFree(copy);
  return list;
}

char* listToDelimitedString(list_t* list, char* delimiter) {
  if (list == NULL) {
    return NULL;
  }
  if (list->len == 0) {
    return oidc_strcopy("");
  }
  list_node_t* node = list_at(list, 0);
  char*        tmp  = NULL;
  char*        str  = oidc_sprintf("%s", (char*)node->val);
  unsigned int i;
  for (i = 1; i < list->len; i++) {
    tmp = oidc_sprintf("%s%s%s", str, delimiter, (char*)list_at(list, i)->val);
    secFree(str);
    if (tmp == NULL) {
      return NULL;
    }
    str = tmp;
  }
  return str;
}

const char** listToArray(list_t* list) {
  if (list == NULL) {
    return NULL;
  }
  if (list->len == 0) {
    return NULL;
  }
  const char** arr =
      secAlloc(sizeof(const char*) *
               (list->len + 1));  // the +1 will add a NULL pointer to the end
  for (unsigned int i = 0; i < list->len; i++) {
    arr[i] = (const char*)list_at(list, (int)i)->val;
  }
  return arr;
}

void* passThrough(void* ptr) { return ptr; }

list_t* createList(int copyValues, char* s, ...) {
  list_t* list                = list_new();
  list->match                 = (matchFunction)strequal;
  void* (*value_f_ptr)(void*) = passThrough;
  if (copyValues) {
    value_f_ptr = (void* (*)(void*))oidc_strcopy;
    list->free  = (void (*)(void*))_secFree;
  }
  if (s == NULL) {
    return list;
  }
  va_list args;
  va_start(args, s);
  list_rpush(list, list_node_new(value_f_ptr(s)));
  char* a;
  while ((a = va_arg(args, char*)) != NULL) {
    list_rpush(list, list_node_new(value_f_ptr(a)));
  }
  va_end(args);
  return list;
}

list_t* intersectLists(list_t* a, list_t* b) {
  list_t* l = list_new();
  l->free   = _secFree;
  l->match  = a->match;
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

list_t* copyList(const list_t* a) {
  list_t* l = list_new();
  l->free   = _secFree;
  l->match  = a->match;
  list_node_t*     node;
  list_iterator_t* it = list_iterator_new((list_t*)a, LIST_HEAD);
  while ((node = list_iterator_next(it))) {
    list_rpush(l, list_node_new(oidc_strcopy(node->val)));
  }
  list_iterator_destroy(it);
  return l;
}

list_t* mergeLists(list_t* a, list_t* b) {
  list_t*          l = copyList(a);
  list_node_t*     node;
  list_iterator_t* it = list_iterator_new(b, LIST_HEAD);
  while ((node = list_iterator_next(it))) {
    list_node_t* n = findInList(a, node->val);
    if (n == NULL) {
      list_rpush(l, list_node_new(oidc_strcopy(node->val)));
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
  l->free   = a->free;
  l->match  = a->match;
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

int listValid(const list_t* l) {
  if (l == NULL) {
    return 0;
  }
  if (l->len == 0) {
    return 0;
  }
  return 1;
}

char* subtractListStrings(const char* a, const char* b, const char del) {
  list_t* al = delimitedStringToList(a, del);
  list_t* bl = delimitedStringToList(b, del);
  list_t* l  = subtractLists(al, bl);
  secFreeList(al);
  secFreeList(bl);
  if (!listValid(l)) {
    secFreeList(l);
    return NULL;
  }
  char* del_str = (char[2]){del, 0};
  char* s       = listToDelimitedString(l, del_str);
  secFreeList(l);
  return s;
}

list_node_t* findInList(list_t* l, const void* v) {
  if (l == NULL) {
    return NULL;
  }
  return list_find(l, (void*)v);
}

list_t* findAllInList(list_t* l, const void* v) {
  if (l == NULL || v == NULL) {
    return NULL;
  }
  list_t* founds = list_new();
  founds->match  = l->match;
  // Don't copy the free function over. We copy the same value pointer, the
  // values should not be freed, only the list
  list_iterator_t* it = list_iterator_new(l, LIST_HEAD);
  list_node_t*     node;
  while ((node = list_iterator_next(it))) {
    if (l->match) {
      if (l->match((void*)v, node->val)) {
        list_rpush(founds, list_node_new(node->val));
      }
    } else {
      if (v == node->val) {
        list_rpush(founds, list_node_new(node->val));
      }
    }
  }
  list_iterator_destroy(it);
  if (!listValid(founds)) {
    secFreeList(founds);
    founds = NULL;
  }
  return founds;
}

void list_removeIfFound(list_t* l, const void* v) {
  if (l == NULL || v == NULL) {
    return;
  }
  list_node_t* node = findInList(l, v);
  if (node == NULL) {
    return;
  }
  return list_remove(l, node);
}

// Merges two subarrays of arr[].
// First subarray is arr[l..m]
// Second subarray is arr[m+1..r]
void _merge(void* arr[], unsigned int l, unsigned int m, unsigned int r,
            matchFunction comp) {
  unsigned int i, j, k;
  unsigned int n1 = m - l + 1;
  unsigned int n2 = r - m;

  /* create temp arrays */
  void* L[n1];
  void* R[n2];

  /* Copy data to temp arrays L[] and R[] */
  for (i = 0; i < n1; i++) L[i] = arr[l + i];
  for (j = 0; j < n2; j++) R[j] = arr[m + 1 + j];

  /* Merge the temp arrays back into arr[l..r]*/
  i = 0;  // Initial index of first subarray
  j = 0;  // Initial index of second subarray
  k = l;  // Initial index of merged subarray
  while (i < n1 && j < n2) {
    if (comp(L[i], R[j]) <= 0) {
      arr[k] = L[i];
      i++;
    } else {
      arr[k] = R[j];
      j++;
    }
    k++;
  }

  /* Copy the remaining elements of L[], if there
  are any */
  while (i < n1) {
    arr[k] = L[i];
    i++;
    k++;
  }

  /* Copy the remaining elements of R[], if there
  are any */
  while (j < n2) {
    arr[k] = R[j];
    j++;
    k++;
  }
}

/* l is for left index and r is right index of the
sub-array of arr to be sorted */
void mergeSort(void* arr[], unsigned int l, unsigned int r,
               matchFunction comp) {
  if (l < r) {
    // Same as (l+r)/2, but avoids overflow for
    // large l and h
    unsigned int m = l + (r - l) / 2;

    // Sort first and second halves
    mergeSort(arr, l, m, comp);
    mergeSort(arr, m + 1, r, comp);

    _merge(arr, l, m, r, comp);
  }
}

void list_mergeSort(list_t* l, matchFunction comp) {
  void* arr[l->len];
  for (size_t i = 0; i < l->len; i++) { arr[i] = list_ats(l, i)->val; }
  mergeSort(arr, 0, l->len - 1, comp);
  for (size_t i = 0; i < l->len; i++) { list_ats(l, i)->val = arr[i]; }
}

void _secFreeList(list_t* l) {
  if (l == NULL) {
    return;
  }
  list_destroy(l);
}

list_t* list_addStringIfNotFound(list_t* l, char* v) {
  if (v == NULL || l == NULL) {
    return l;
  }
  if (findInList(l, v)) {
    return l;
  }
  char* value = v;
  if (l->free == _secFree) {
    value = oidc_strcopy(v);
  }
  list_rpush(l, list_node_new(value));
  return l;
}

void _printList(list_t* l) {
  if (l == NULL) {
    printStdout("NULL\n");
    return;
  }
  printStdout("LIST START\n");
  list_node_t*     node;
  list_iterator_t* it = list_iterator_new(l, LIST_HEAD);
  while ((node = list_iterator_next(it))) {
    printStdout("\tnode value: %s\n", (char*)node->val ?: "NULL");
  }
  list_iterator_destroy(it);
  printStdout("LIST END\n");
}

list_node_t* list_ats(list_t* l, size_t pos) { return list_at(l, (int)pos); }