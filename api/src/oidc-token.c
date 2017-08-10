#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>
#include <ctype.h>
#include <getopt.h>

#include "oidc-token.h"
#include "api.h"


/** @fn int main(int argc, char** argv)
 * @brief gets an access token from the oidc daemon
 * @param argc the number of command line arguments
 * @param argv the command line arguments. The provider name shall be provided
 * with -c
 * @return 0 on success, otherwise an error code
 */
int main(int argc, char** argv) {
  openlog("oidc-token-client", LOG_CONS|LOG_PID, LOG_AUTHPRIV);
  setlogmask(LOG_UPTO(LOG_DEBUG));
  int l_flag = 0;
  unsigned long min_valid_period = 0;
  char* provider = NULL;
  int c;
  while ((c = getopt (argc, argv, "p:t:l")) != -1)
    switch (c) {
      case 'p':
        provider = optarg;
        break;
      case 'l':
        l_flag++;
        break;
      case 't':
        if(!isdigit(*optarg)) {
          fprintf(stderr, "option -t requires an numeric argument");
          exit(EXIT_FAILURE);
        }
        min_valid_period = atoi(optarg);
        break;
      case '?':
        if (optopt == 'p')
          fprintf(stderr, "Option -%c requires an argument.\n", optopt);
        else if (optopt == 't')
          fprintf(stderr, "Option -%c requires an argument.\n", optopt);
        else if (isprint (optopt))
          fprintf(stderr, "Unknown option `-%c'.\n", optopt);
        else
          fprintf(stderr, "Unknown option character `\\x%x'.\n", optopt);
        exit(EXIT_FAILURE);
      default:
        abort ();
    }

  if(l_flag) {
    char* providerList = getLoadedProvider();
    printf("The following providers are configured: %s\n", providerList);
    free(providerList);
  }
  if(provider) {
    char* access_token = getAccessToken(provider, min_valid_period);
    printf("%s\n",access_token ? access_token : "No access token.");
    free(access_token);
  }
  return EXIT_SUCCESS;
}
