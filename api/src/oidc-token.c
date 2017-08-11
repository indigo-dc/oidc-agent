#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>
#include <ctype.h>
#include <argp.h>

#include "oidc-token.h"
#include "api.h"


const char *argp_program_version = "oidc-token 0.1.0";

const char *argp_program_bug_address = "<gabriel.zachmann@kit.edu>";

/* This structure is used by main to communicate with parse_opt. */
struct arguments {
  char* args[1];            /* provider */
  int list_provider;
  unsigned long min_valid_period;  /* Arguments for -t */
};

/*
   OPTIONS.  Field 1 in ARGP.
   Order of fields: {NAME, KEY, ARG, FLAGS, DOC}.
*/
static struct argp_option options[] = {
  {"listprovider", 'l', 0, 0, "Lists the currently loaded providers", 0},
  {"time",  't', "min_valid_period", 0, "period of how long the access token should be at least valid in seconds", 0},
  {0}
};

/*
   PARSER. Field 2 in ARGP.
   Order of parameters: KEY, ARG, STATE.
*/
static error_t parse_opt (int key, char *arg, struct argp_state *state) {
  struct arguments *arguments = state->input;

  switch (key)
  {
    case 'l':
      arguments->list_provider = 1;
      break;
    case 't':
      if(!isdigit(*arg)) 
        return ARGP_ERR_UNKNOWN;
      arguments->min_valid_period = atoi(arg);
      break;
    case ARGP_KEY_ARG:
      if (state->arg_num >= 1)
        argp_usage(state);
      arguments->args[state->arg_num] = arg;
      break;
    case ARGP_KEY_END:
      if(arguments->list_provider)
        break;
      if (state->arg_num < 1)
        argp_usage (state);
      break;
    default:
      return ARGP_ERR_UNKNOWN;
  }
  return 0;
}

/*
   ARGS_DOC. Field 3 in ARGP.
   A description of the non-option command-line arguments
     that we accept.
*/
static char args_doc[] = "PROVIDER_SHORTNAME | -l";

/*
  DOC.  Field 4 in ARGP.
  Program documentation.
*/
static char doc[] = "oidc-token -- A client for oidcd for getting oidcd access tokens.";

/*
   The ARGP structure itself.
*/
static struct argp argp = {options, parse_opt, args_doc, doc};



int main (int argc, char **argv) {
  struct arguments arguments;
  
  /* Set argument defaults */
  arguments.min_valid_period = 0;
  arguments.list_provider = 0;
  arguments.args[0]=NULL;

  argp_parse (&argp, argc, argv, 0, 0, &arguments);


  if(arguments.list_provider) {
    char* providerList = getLoadedProvider();
    printf("The following providers are configured: %s\n", providerList);
    free(providerList);
  }
  if(arguments.args[0]) {
    char* access_token = getAccessToken(arguments.args[0], arguments.min_valid_period);
    printf("%s\n",access_token ? access_token : "No access token.");
    free(access_token);
  }

  return 0;
}











//
//
//
// /** @fn int main(int argc, char** argv)
//  * @brief gets an access token from the oidc daemon
//  * @param argc the number of command line arguments
//  * @param argv the command line arguments. The provider name shall be provided
//  * with -c
//  * @return 0 on success, otherwise an error code
//  */
// int main(int argc, char** argv) {
//   openlog("oidc-token-client", LOG_CONS|LOG_PID, LOG_AUTHPRIV);
//   // setlogmask(LOG_UPTO(LOG_DEBUG));
//   setlogmask(LOG_UPTO(LOG_NOTICE));
//   int l_flag = 0;
//   unsigned long min_valid_period = 0;
//   char* provider = NULL;
//   int c;
//   while ((c = getopt (argc, argv, "p:t:l")) != -1)
//     switch (c) {
//       case 'p':
//         provider = optarg;
//         break;
//       case 'l':
//         l_flag++;
//         break;
//       case 't':
//         if(!isdigit(*optarg)) {
//           fprintf(stderr, "option -t requires an numeric argument");
//           exit(EXIT_FAILURE);
//         }
//         min_valid_period = atoi(optarg);
//         break;
//       case '?':
//         if (optopt == 'p')
//           fprintf(stderr, "Option -%c requires an argument.\n", optopt);
//         else if (optopt == 't')
//           fprintf(stderr, "Option -%c requires an argument.\n", optopt);
//         else if (isprint (optopt))
//           fprintf(stderr, "Unknown option `-%c'.\n", optopt);
//         else
//           fprintf(stderr, "Unknown option character `\\x%x'.\n", optopt);
//         exit(EXIT_FAILURE);
//       default:
//         abort ();
//     }
//
//   if(l_flag) {
//     char* providerList = getLoadedProvider();
//     printf("The following providers are configured: %s\n", providerList);
//     free(providerList);
//   }
//   if(provider) {
//     char* access_token = getAccessToken(provider, min_valid_period);
//     printf("%s\n",access_token ? access_token : "No access token.");
//     free(access_token);
//   }
//   return EXIT_SUCCESS;
// }
