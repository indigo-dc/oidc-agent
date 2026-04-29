#ifndef ERROR_UTILS_H
#define ERROR_UTILS_H

char* combineError(const char* error, const char* error_description);
int   matchError(const char* error, const char* errorcode);

#endif  // ERROR_UTILS_H
