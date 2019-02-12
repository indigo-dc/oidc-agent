#include "commonFeatures.h"
#include "list/list.h"
#include "utils/file_io/fileUtils.h"
#include "utils/listUtils.h"
#include "utils/memory.h"
#include "utils/printer.h"

void common_handleListAccountConfigs() {
  list_t* list = getAccountConfigFileList();
  list_mergeSort(list, (int (*)(const void*, const void*))compareFilesByName);
  char* str = listToDelimitedString(list, '\n');
  list_destroy(list);
  printf("The following account configurations are usable: \n%s\n", str);
  secFree(str);
}
