#ifndef PRINTER_UTILS_H
#define PRINTER_UTILS_H

#include <stdio.h>

void printEnvs(char* daemon_socket, pid_t daemon_pid, unsigned char quiet, unsigned char json);

#endif  // PRINTER_H
