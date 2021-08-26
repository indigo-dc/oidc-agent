#ifndef REGISTRY_CONNECTOR_H
#define REGISTRY_CONNECTOR_H

#include <windows.h>
#include <stdio.h>

void createOrUpdateRegistryEntry(const char* key, const char* value);
void getRegistryEntry(const char* key, const char *value);
void removeRegistryEntry(const char* value);

#endif  