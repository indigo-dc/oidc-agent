#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libconfig.h>

#include "../lib/jsmn.h"

#include "config.h"
#include "http.h"


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
  if(config_lookup_string(&cfg, "configuration_endpoint", &config.configuration_endpoint))
    printf("configuration_endpoint: %s\n", config.configuration_endpoint);
  else
    fprintf(stderr, "No 'configuration_endpoint' setting in configuration file.\n");
  if(config_lookup_string(&cfg, "refresh_token", &config.refresh_token))
    printf("refresh_token: %s\n", config.refresh_token);
  else
    printf("No refresh_token");
  if(config_lookup_string(&cfg, "cert_path", &config.cert_path))
    printf("cert_path: %s\n", config.cert_path);
  else {
    config.cert_path = "/etc/ssl/certs";
    printf("WARNING: No cert_path found in config file. Setting default cert_path: %s\n", config.cert_path);
  }

  getOIDCProviderConfig();
}

void getOIDCProviderConfig() {
  const char* res = httpsGET(config.configuration_endpoint);

  int i;
	int r;
	jsmn_parser p;
  int token_needed = jsmn_parse(&p, res, strlen(res), NULL, 0);
  printf("%d\n",token_needed);
	jsmntok_t t[token_needed+6]; 
	jsmn_init(&p);
	r = jsmn_parse(&p, res, strlen(res), t, sizeof(t)/sizeof(t[0]));
	if (r < 0) {
		fprintf(stderr, "Failed to parse JSON: %d\n", r);
    exit(EXIT_FAILURE);
	}

	/* Assume the top-level element is an object */
	if (r < 1 || t[0].type != JSMN_OBJECT) {
		fprintf(stderr, "Object expected\n");
    exit(EXIT_FAILURE);
	}

	/* Loop over all keys of the root object */
	for (i = 1; i < r; i++) {
		if (jsoneq(res, &t[i], "token_endpoint") == 0) {
			/* We may use strndup() to fetch string value */
      config.token_endpoint = (char*) malloc(t[i+1].end-t[i+1].start);
			sprintf(config.token_endpoint,"%.*s", t[i+1].end-t[i+1].start,
					res + t[i+1].start);
			i++;
      break;
		} else {
			//printf("Unexpected key: %.*s\n", t[i].end-t[i].start,res + t[i].start);
		}
	}
}

static int jsoneq(const char *json, jsmntok_t *tok, const char *s) {
    if (tok->type == JSMN_STRING && (int) strlen(s) == tok->end - tok->start &&
        strncmp(json + tok->start, s, tok->end - tok->start) == 0) {
          return 0;
            
    }
      return -1;

}
