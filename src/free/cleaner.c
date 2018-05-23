#include "cleaner.h"

#include "../oidc_utilities.h" //TODO remove when not longer needed

void clearFreeStringArray(char** arr, size_t size) {
  size_t i;
  for(i=0; i<size; i++) {
    clearFreeString(arr[i]);
  }
  clearFree(arr, size * sizeof(char*));
}
