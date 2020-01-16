#ifndef PRINTER_H
#define PRINTER_H

#include <stdio.h>

int fprintNormal(FILE* out, char* fmt, ...);
int printNormal(char* fmt, ...);
int printStdout(char* fmt, ...);
int printError(char* fmt, ...);
int printPrompt(char* fmt, ...);
int printImportant(char* fmt, ...);

#endif  // PRINTER_H
