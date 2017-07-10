#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "http.h"

#define CONFIGFILE "config.conf"

#define LOG_DEBUG


int main(int argc, char** argv) {
  readConfig(CONFIGFILE);
  const char* res = httpsGET("https://iam-test.indigo-datacloud.eu/.well-known/openid-configuration");
  return 0;
}

