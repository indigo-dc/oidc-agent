#ifndef PORT_UTILS_H
#define PORT_UTILS_H

unsigned short getRandomPort();
char*          portToUri(unsigned short port);
unsigned short getPortFromUri(const char* uri);

#endif  // PORT_UTILS_H
