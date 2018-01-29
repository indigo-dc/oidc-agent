#define _XOPEN_SOURCE 500

#include "parse_ipc.h"
#include "json.h"
#include "settings.h"
#include "oidc_error.h"
#include "ipc_values.h"
#include "gen_handler.h"
#include "oidc_utilities.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <syslog.h>
#include <strings.h>

/**
 * @param res a pointer to the response that should be parsed. The pointer will
 * be freed!
 */
char* gen_parseResponse(char* res, int verbose) {
  struct key_value pairs[6];
  pairs[0].key = "status";
  pairs[1].key = "config";
  pairs[2].key = "error";
  pairs[3].key = "uri";
  pairs[4].key = "info";
  pairs[5].key = "state";
  if(getJSONValues(res, pairs, sizeof(pairs)/sizeof(*pairs))<0) {
    printError("Could not decode json: %s\n", res);
    printError("This seems to be a bug. Please hand in a bug report.\n");
    clearFreeString(res);
    exit(EXIT_FAILURE);
  }
  clearFreeString(res);
  if(pairs[2].value!=NULL) {
    printError("Error: %s\n", pairs[2].value);
    clearFreeKeyValuePairs(pairs, sizeof(pairs)/sizeof(*pairs));
    exit(EXIT_FAILURE);
  }
  char* config = NULL;
  if(pairs[1].value!=NULL) {
    config = pairs[1].value;
  } else {
    if(strcasecmp(pairs[0].value, STATUS_NOTFOUND)==0) {
      syslog(LOG_AUTHPRIV|LOG_DEBUG, "%s", pairs[4].value);
      clearFreeKeyValuePairs(pairs, sizeof(pairs)/sizeof(*pairs));
      return NULL;
    }
    if(pairs[3].value==NULL){
      printError("Error: response does not contain updated config\n");
    }
  }
  printf("%s\n", pairs[0].value);
  if(strcmp(pairs[0].value, STATUS_SUCCESS)==0) {
    printf("The generated account config was successfully added to oidc-agent. You don't have to run oidc-add.\n");
  } else if(strcasecmp(pairs[0].value, STATUS_ACCEPTED)==0) {
    if(pairs[4].value) {
      printf(C_IMPORTANT "%s\n" C_RESET, pairs[4].value);
    }
    if(pairs[3].value) {
      printf(C_IMPORTANT "To continue the account generation process visit the following URL in a Browser of your choice:\n%s\n" C_RESET, pairs[3].value);
      char* cmd = oidc_sprintf("xdg-open \"%s\"", pairs[3].value);
      system(cmd);
      clearFreeString(cmd);
    }
    if(pairs[5].value) {
      usleep(2*1000*1000);
      handleStateLookUp(pairs[5].value, verbose);
    }
    clearFreeKeyValuePairs(pairs, sizeof(pairs)/sizeof(*pairs));
  } 
  clearFreeString(pairs[0].value); clearFreeKeyValuePairs(&pairs[2], sizeof(pairs)/sizeof(*pairs)-2);
  return config;
}

void add_parseResponse(char* res) {
  if(NULL==res) {
    printError("Error: %s\n", oidc_serror());
    exit(EXIT_FAILURE);
  }

  struct key_value pairs[2];
  pairs[0].key = "status";
  pairs[1].key = "error";
  if(getJSONValues(res, pairs, sizeof(pairs)/sizeof(*pairs))<0) {
    printError("Could not decode json: %s\n", res);
    printError("This seems to be a bug. Please hand in a bug report.\n");
    clearFreeString(res);
    exit(EXIT_FAILURE);
  }
  clearFreeString(res);
  if(pairs[1].value!=NULL) {
    printError("Error: %s\n", pairs[1].value);
    clearFreeString(pairs[1].value); clearFreeString(pairs[0].value);
    exit(EXIT_FAILURE);
  }
  printf("%s\n", pairs[0].value);
  clearFreeString(pairs[0].value);
}
