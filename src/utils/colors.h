#ifndef OIDC_COLORS_H
#define OIDC_COLORS_H

#define C_RED "\x1B[31m"
#define C_GRN "\x1B[32m"
#define C_YEL "\x1B[33m"
#define C_BLU "\x1B[34m"
#define C_MAG "\x1B[35m"
#define C_CYN "\x1B[36m"
#define C_WHT "\x1B[37m"
#define C_RESET "\x1B[0m"

// Setting colors
#ifndef C_ERROR
#define C_ERROR C_RED
#endif
#ifndef C_PROMPT
#define C_PROMPT C_CYN
#endif
#ifndef C_IMPORTANT
#define C_IMPORTANT C_YEL
#endif

#include <stdarg.h>

int printErrorColored(char* fmt, va_list args);
int printPromptColored(char* fmt, va_list args);
int printImportantColored(char* fmt, va_list args);

#endif  // OIDC_COLORS_H
