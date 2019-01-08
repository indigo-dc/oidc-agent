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

list_t* delimitedStringToList(const char* str, char delimiter) {
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
    str = oidc_sprintf("%s", (char*)node->val);
  }
  unsigned int i;
  for (i = 1; i < list->len; i++) {
    tmp = oidc_sprintf("%s%c%s", str, delimiter, (char*)list_at(list, i)->val);
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
    list_node_t* n = list_find(b, node->val);
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
    list_node_t* n = list_find(b, node->val);
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

void list_removeIfFound(list_t* l, void* v) {
  if (l == NULL || v == NULL) {
    return;
  }
  list_node_t* node = list_find(l, v);
  if (node == NULL) {
    return;
  }
  return list_remove(l, node);
}

// Merges two subarrays of arr[].
// First subarray is arr[l..m]
// Second subarray is arr[m+1..r]
void _merge(void* arr[], int l, int m, int r,
            int (*comp)(const void*, const void*)) {
  int i, j, k;
  int n1 = m - l + 1;
  int n2 = r - m;

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
void mergeSort(void* arr[], int l, int r,
               int (*comp)(const void*, const void*)) {
  if (l < r) {
    // Same as (l+r)/2, but avoids overflow for
    // large l and h
    int m = l + (r - l) / 2;

    // Sort first and second halves
    mergeSort(arr, l, m, comp);
    mergeSort(arr, m + 1, r, comp);

    _merge(arr, l, m, r, comp);
  }
}

void list_mergeSort(list_t* l, int (*comp)(const void*, const void*)) {
  void* arr[l->len];
  for (size_t i = 0; i < l->len; i++) { arr[i] = list_at(l, i)->val; }
  mergeSort(arr, 0, l->len - 1, comp);
  for (size_t i = 0; i < l->len; i++) { list_at(l, i)->val = arr[i]; }
}

void secFreeList(list_t* l) {
  if (l == NULL) {
    return;
  }
  list_destroy(l);
}
