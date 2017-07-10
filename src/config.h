#ifndef CONFIG_H
#define CONFIG_H


struct {
  const char* client_id;
  const char* client_secret;
} config;

void readConfig(const char* filename);

#endif //CONFIG_H
