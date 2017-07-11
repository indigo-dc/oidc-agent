#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "oidc.h"

#define CONFIGFILE "config.conf"

#define LOG_DEBUG


int main(int argc, char** argv) {
  readConfig(CONFIGFILE);
  refreshToken();

  return 0;
}

