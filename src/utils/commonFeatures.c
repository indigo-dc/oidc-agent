#include "commonFeatures.h"

#include "defines/ipc_values.h"
#include "ipc/cryptCommunicator.h"
#include "utils/file_io/fileUtils.h"
#include "utils/listUtils.h"
#include "utils/memory.h"
#include "utils/oidc_error.h"
#include "utils/printer.h"
#include "utils/string/stringUtils.h"
#include "utils/system_runner.h"

void common_handleListConfiguredAccountConfigs() {
  list_t* list = getAccountConfigFileList();
  if (list == NULL) {
    if (oidc_errno != OIDC_SUCCESS) {
      oidc_perror();
      exit(EXIT_FAILURE);
    }
    printStdoutIfTTY("No account configurations created yet.\n");
    exit(EXIT_SUCCESS);
  }
  list_mergeSort(list, (matchFunction)compareFilesByName);
  char* str = listToDelimitedString(list, "\n");
  secFreeList(list);
  printStdoutIfTTY("The following account configurations are usable: \n");
  printStdout("%s\n", str);
  secFree(str);
}

void common_assertAgent(unsigned char remote) {
  char* res = ipc_cryptCommunicate(remote, REQUEST_CHECK);
  if (res == NULL) {
    oidc_perror();
    exit(EXIT_FAILURE);
  }
  secFree(res);
}

void common_assertOidcPrompt() {
  char* ret = getOutputFromCommand("oidc-prompt --version");
  if (!strValid(ret)) {
    printError("oidc-prompt not installed. To use GUI prompting please install "
               "the 'oidc-agent-prompt' package.\n");
    exit(EXIT_FAILURE);
  }
  secFree(ret);
}
