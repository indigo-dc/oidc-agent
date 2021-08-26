#include <windows.h>
#include <stdio.h>
#include "defines/settings.h"


void createOrUpdateRegistryEntry(const char* key, const char* value) {
    HKEY createdKey; 
	if (RegCreateKey(HKEY_CURRENT_USER, OIDC_AGENT_REGISTRY, &createdKey) != ERROR_SUCCESS) {
		printf("Could not read registry value!\n");
	}
	if (RegSetValueEx(createdKey, key, 0, REG_SZ, (LPBYTE)value, strlen(value)*sizeof(char)) != ERROR_SUCCESS) {
		printf("Could not write registry value!\n");
	}
}

void getRegistryEntry(const char* key, const char* value) {
    DWORD BufferSize = 8192;
    RegGetValue(HKEY_CURRENT_USER, OIDC_AGENT_REGISTRY, key, RRF_RT_ANY, NULL, (PVOID)value, &BufferSize);
}

void removeRegistryEntry(const char* value) {
    RegDeleteKeyValue(HKEY_CURRENT_USER, OIDC_AGENT_REGISTRY, value);
}