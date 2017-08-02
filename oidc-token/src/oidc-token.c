#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>
#include <ctype.h>

#include "oidc-token.h"
#include "../../src/ipc.h"

char* provider = NULL;

/** @fn int main(int argc, char** argv)
 * @brief gets an access token from the oidc daemon
 * @param argc the number of command line arguments
 * @param argv the command line arguments. The provider name shall be provided
 * with -c
 * @return 0 on success, otherwise an error code
 */
int main(int argc, char** argv) {
  openlog("oidc-token", LOG_CONS|LOG_PID, LOG_AUTHPRIV);
  setlogmask(LOG_UPTO(LOG_DEBUG));
  // setlogmask(LOG_UPTO(LOG_NOTICE));
  parseOpt(argc, argv);
  if(provider==NULL) {
    printf("You have to specify a provider\n");
    exit(EXIT_SUCCESS);
  }
  static struct connection con;
  if(ipc_init(&con, "token", "OIDC_TOKEN_SOCKET_PATH", 0)==DAEMON_NOT_RUNNING) {
    syslog(LOG_AUTHPRIV|LOG_ERR, "Could not init socket, because env var not set. Daemon not running.");
    printf("Daemon not running. Please start the daemon first.");
    return EXIT_SUCCESS;
  }
  int sock = ipc_connect(con);
  if(sock<0) {
    syslog(LOG_AUTHPRIV|LOG_ALERT, "Could not connect to socket");
    ipc_close(con);
    return EXIT_FAILURE;
  }
  ipc_write(sock, provider);
  char* access_token = ipc_read(sock);
  printf("%s\n",access_token);
  free(access_token);
  ipc_close(con);
  return EXIT_SUCCESS;
}

/** @fn void parseOpt(int argc, char* const* argv)
 * @brief parses the command line arguments
 * @param argc the number of command line arguments
 * @param argv the command line arguments
 */
void parseOpt(int argc, char* const* argv) {
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "Parsing arguments");
  int c;
  while ((c = getopt (argc, argv, "c:")) != -1)
    switch (c) {
      case 'c':
        provider = optarg;
        break;
      case '?':
        if (optopt == 'c')
          syslog(LOG_AUTHPRIV|LOG_ERR, "Option -%c requires an argument.\n", optopt);
        else if (isprint (optopt))
          syslog(LOG_AUTHPRIV|LOG_ERR, "Unknown option `-%c'.\n", optopt);
        else
          syslog(LOG_AUTHPRIV|LOG_ERR, "Unknown option character `\\x%x'.\n", optopt);
        exit(EXIT_FAILURE);
      default:
        abort ();
    }
}
