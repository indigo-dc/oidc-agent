#include "privileges.h"

#include <seccomp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "defines/settings.h"
#include "utils/file_io/file_io.h"
#include "utils/listUtils.h"
#include "utils/memory.h"
#include "utils/oidc_error.h"
#include "utils/printer.h"
#include "utils/string/stringUtils.h"
#include "wrapper/list.h"

void checkRc(int rc, const char* str, const char* syscall) {
  if (rc < 0) {
    perror(str);
    printError("On syscall: %s\n", syscall);
    exit(-rc);
  }
}

void addSysCallsFromConfigFile(scmp_filter_ctx ctx, const char* path) {
  list_t* lines = getLinesFromFile(path);
  if (lines == NULL) {
    oidc_errno = OIDC_ENOPRIVCONF;
    oidc_perror();
    exit(EXIT_FAILURE);
  }
  list_node_t*     node;
  list_iterator_t* it = list_iterator_new(lines, LIST_HEAD);
  while ((node = list_iterator_next(it))) {
    char* syscall = node->val;
    ALLOW_SYSCALL(ctx, strtok(syscall, " "));
  }
  list_iterator_destroy(it);
  secFreeList(lines);
}

void addSocketSysCalls(scmp_filter_ctx ctx) {
  char* path = oidc_sprintf("%s/%s.priv", PRIVILEGES_PATH, "socket");
  addSysCallsFromConfigFile(ctx, path);
  secFree(path);
}

void addFileReadSysCalls(scmp_filter_ctx ctx) {
  char* path = oidc_sprintf("%s/%s.priv", PRIVILEGES_PATH, "read");
  addSysCallsFromConfigFile(ctx, path);
  secFree(path);
}

void addCryptSysCalls(scmp_filter_ctx ctx) {
  char* path = oidc_sprintf("%s/%s.priv", PRIVILEGES_PATH, "crypt");
  addSysCallsFromConfigFile(ctx, path);
  secFree(path);
}

void addAgentIpcSysCalls(scmp_filter_ctx ctx) {
  char* path = oidc_sprintf("%s/%s.priv", PRIVILEGES_PATH, "agentIpc");
  addSysCallsFromConfigFile(ctx, path);
  secFree(path);
}

void addDaemonSysCalls(scmp_filter_ctx ctx) {
  char* path = oidc_sprintf("%s/%s.priv", PRIVILEGES_PATH, "daemon");
  addSysCallsFromConfigFile(ctx, path);
  secFree(path);
}

void addKillSysCall(scmp_filter_ctx ctx) {
  char* path = oidc_sprintf("%s/%s.priv", PRIVILEGES_PATH, "kill");
  addSysCallsFromConfigFile(ctx, path);
  secFree(path);
}

void addHttpSysCalls(scmp_filter_ctx ctx) {
  char* path = oidc_sprintf("%s/%s.priv", PRIVILEGES_PATH, "http");
  addSysCallsFromConfigFile(ctx, path);
  secFree(path);
}

void addHttpServerSysCalls(scmp_filter_ctx ctx) {
  char* path = oidc_sprintf("%s/%s.priv", PRIVILEGES_PATH, "httpserver");
  addSysCallsFromConfigFile(ctx, path);
  secFree(path);
}

void addFileWriteSysCalls(scmp_filter_ctx ctx) {
  addFileReadSysCalls(ctx);
  char* path = oidc_sprintf("%s/%s.priv", PRIVILEGES_PATH, "write");
  addSysCallsFromConfigFile(ctx, path);
  secFree(path);
}

void addTimeSysCalls(scmp_filter_ctx ctx) {
  // ALLOW_SYSCALL_PARAM(ctx, open,
  //                     SCMP_A0(SCMP_CMP_EQ, (scmp_datum_t)
  //                     "/etc/localtime"));

  char* path = oidc_sprintf("%s/%s.priv", PRIVILEGES_PATH, "time");
  addSysCallsFromConfigFile(ctx, path);
  secFree(path);
}

void addLoggingSysCalls(scmp_filter_ctx ctx) {
  addTimeSysCalls(ctx);
  char* path = oidc_sprintf("%s/%s.priv", PRIVILEGES_PATH, "logging");
  addSysCallsFromConfigFile(ctx, path);
  secFree(path);
}

void addPromptingSysCalls(scmp_filter_ctx ctx) {
  addPrintingSysCalls(ctx);
  char* path = oidc_sprintf("%s/%s.priv", PRIVILEGES_PATH, "prompt");
  addSysCallsFromConfigFile(ctx, path);
  secFree(path);
}

void addPrintingSysCalls(scmp_filter_ctx ctx) {
  char* path = oidc_sprintf("%s/%s.priv", PRIVILEGES_PATH, "print");
  addSysCallsFromConfigFile(ctx, path);
  secFree(path);
}

void addMemorySysCalls(scmp_filter_ctx ctx) {
  char* path = oidc_sprintf("%s/%s.priv", PRIVILEGES_PATH, "memory");
  addSysCallsFromConfigFile(ctx, path);
  secFree(path);
}

void addGeneralSysCalls(scmp_filter_ctx ctx) {
  char* path = oidc_sprintf("%s/%s.priv", PRIVILEGES_PATH, "general");
  addSysCallsFromConfigFile(ctx, path);
  secFree(path);
  addMemorySysCalls(ctx);
  addPrintingSysCalls(ctx);
}

void addSignalHandlingSysCalls(scmp_filter_ctx ctx) {
  char* path = oidc_sprintf("%s/%s.priv", PRIVILEGES_PATH, "signal");
  addSysCallsFromConfigFile(ctx, path);
  secFree(path);
}

void addSleepSysCalls(scmp_filter_ctx ctx) {
  char* path = oidc_sprintf("%s/%s.priv", PRIVILEGES_PATH, "sleep");
  addSysCallsFromConfigFile(ctx, path);
  secFree(path);
}
