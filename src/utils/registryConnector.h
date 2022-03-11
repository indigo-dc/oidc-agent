#ifndef REGISTRY_CONNECTOR_H
#define REGISTRY_CONNECTOR_H

#include <windows.h>
#include <stdio.h>

void createOrUpdateRegistryEntry(const char* key, const char* value);
char* getRegistryValue(const char* env_name);
void removeRegistryEntry(const char* value);

#endif  