#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libconfig.h>

#include "config.h"


void readConfig(const char* filename) {
  config_t cfg;
  config_setting_t *setting;
  const char *str;

  config_init(&cfg);

  /* Read the file. If there is an error, report it and exit. */
  if(! config_read_file(&cfg, filename))

  {
    fprintf(stderr, "%s:%d - %s\n", config_error_file(&cfg),
        config_error_line(&cfg), config_error_text(&cfg));
    config_destroy(&cfg);
    exit(EXIT_FAILURE);
  }


  if(config_lookup_string(&cfg, "client_id", &config.client_id))
    printf("client_id: %s\n", config.client_id);
  else
    fprintf(stderr, "No 'client_id' setting in configuration file.\n");
  if(config_lookup_string(&cfg, "client_secret", &config.client_secret))
    printf("client_secret: %s\n", config.client_secret);
  else
    fprintf(stderr, "No 'client_secret' setting in configuration file.\n");
}
