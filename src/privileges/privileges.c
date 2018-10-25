#include "privileges.h"
#include "../file_io/file_io.h"

#include "../../lib/list/src/list.h"

#include <seccomp.h>
#include <stdio.h>
#include <stdlib.h>

// #define CONFIG_PATH "/etc/oidc-agent/privileges"
#define CONFIG_PATH "config/privileges"

void checkRc(int rc, const char* str, const char* syscall) {
  if (rc < 0) {
    perror(str);
    printError("On syscall: %s\n", syscall);
    exit(-rc);
  }
}

void addSysCallsFromConfigFile(scmp_filter_ctx ctx, const char* path) {
  list_t*          lines = getLinesFromFile(path);
  list_node_t*     node;
  list_iterator_t* it = list_iterator_new(lines, LIST_HEAD);
  while ((node = list_iterator_next(it))) {
    char* syscall = node->val;
    ALLOW_SYSCALL(ctx, strtok(syscall, " "));
  }
  list_iterator_destroy(it);
  list_destroy(lines);
}

void addSocketSysCalls(scmp_filter_ctx ctx) {
  char* path = oidc_sprintf("%s/%s.priv", CONFIG_PATH, "socket");
  addSysCallsFromConfigFile(ctx, path);
  secFree(path);
}

void addFileReadSysCalls(scmp_filter_ctx ctx) {
  char* path = oidc_sprintf("%s/%s.priv", CONFIG_PATH, "read");
  addSysCallsFromConfigFile(ctx, path);
  secFree(path);
}

void addCryptSysCalls(scmp_filter_ctx ctx) {
  char* path = oidc_sprintf("%s/%s.priv", CONFIG_PATH, "crypt");
  addSysCallsFromConfigFile(ctx, path);
  secFree(path);
}

void addAgentIpcSysCalls(scmp_filter_ctx ctx) {
  char* path = oidc_sprintf("%s/%s.priv", CONFIG_PATH, "agentIpc");
  addSysCallsFromConfigFile(ctx, path);
  secFree(path);
}

void addDaemonSysCalls(scmp_filter_ctx ctx) {
  char* path = oidc_sprintf("%s/%s.priv", CONFIG_PATH, "daemon");
  addSysCallsFromConfigFile(ctx, path);
  secFree(path);
}

void addKillSysCall(scmp_filter_ctx ctx) {
  char* path = oidc_sprintf("%s/%s.priv", CONFIG_PATH, "kill");
  addSysCallsFromConfigFile(ctx, path);
  secFree(path);
}

void addHttpSysCalls(scmp_filter_ctx ctx) {
  char* path = oidc_sprintf("%s/%s.priv", CONFIG_PATH, "http");
  addSysCallsFromConfigFile(ctx, path);
  secFree(path);
}

void addHttpServerSysCalls(scmp_filter_ctx ctx) {
  char* path = oidc_sprintf("%s/%s.priv", CONFIG_PATH, "httpserver");
  addSysCallsFromConfigFile(ctx, path);
  secFree(path);
}

void addFileWriteSysCalls(scmp_filter_ctx ctx) {
  addFileReadSysCalls(ctx);
  char* path = oidc_sprintf("%s/%s.priv", CONFIG_PATH, "write");
  addSysCallsFromConfigFile(ctx, path);
  secFree(path);
}

void addTimeSysCalls(scmp_filter_ctx ctx) {
  // ALLOW_SYSCALL_PARAM(ctx, open,
  //                     SCMP_A0(SCMP_CMP_EQ, (scmp_datum_t)
  //                     "/etc/localtime"));

  char* path = oidc_sprintf("%s/%s.priv", CONFIG_PATH, "time");
  addSysCallsFromConfigFile(ctx, path);
  secFree(path);
}

void addLoggingSysCalls(scmp_filter_ctx ctx) {
  addTimeSysCalls(ctx);
  char* path = oidc_sprintf("%s/%s.priv", CONFIG_PATH, "logging");
  addSysCallsFromConfigFile(ctx, path);
  secFree(path);
}

void addPromptingSysCalls(scmp_filter_ctx ctx) {
  addPrintingSysCalls(ctx);
  char* path = oidc_sprintf("%s/%s.priv", CONFIG_PATH, "prompt");
  addSysCallsFromConfigFile(ctx, path);
  secFree(path);
}

void addPrintingSysCalls(scmp_filter_ctx ctx) {
  char* path = oidc_sprintf("%s/%s.priv", CONFIG_PATH, "print");
  addSysCallsFromConfigFile(ctx, path);
  secFree(path);
}

void addMemorySysCalls(scmp_filter_ctx ctx) {
  char* path = oidc_sprintf("%s/%s.priv", CONFIG_PATH, "memory");
  addSysCallsFromConfigFile(ctx, path);
  secFree(path);
}

void addGeneralSysCalls(scmp_filter_ctx ctx) {
  char* path = oidc_sprintf("%s/%s.priv", CONFIG_PATH, "general");
  addSysCallsFromConfigFile(ctx, path);
  secFree(path);
  addMemorySysCalls(ctx);
  addPrintingSysCalls(ctx);
}

void addSignalHandlingSysCalls(scmp_filter_ctx ctx) {
  char* path = oidc_sprintf("%s/%s.priv", CONFIG_PATH, "signal");
  addSysCallsFromConfigFile(ctx, path);
  secFree(path);
}

void addSleepSysCalls(scmp_filter_ctx ctx) {
  char* path = oidc_sprintf("%s/%s.priv", CONFIG_PATH, "sleep");
  addSysCallsFromConfigFile(ctx, path);
  secFree(path);
}

void addExecSysCalls(scmp_filter_ctx ctx) {
  char* path = oidc_sprintf("%s/%s.priv", CONFIG_PATH, "exec");
  addSysCallsFromConfigFile(ctx, path);
  secFree(path);
}
