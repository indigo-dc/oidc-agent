#ifndef PRINTER_UTILS_H
#define PRINTER_UTILS_H

#include <sys/types.h>

void printEnvs(const char* daemon_socket, pid_t daemon_pid, unsigned char quiet,
               unsigned char json);

#endif  // PRINTER_UTILS_H
