#ifndef KEY_SET_H
#define KEY_SET_H

struct keySetPPstr {
  char* priv;
  char* pub;
};

struct keySetSEstr {
  char* sign;
  char* enc;
};

void _secFreeKeySetSEstr(struct keySetSEstr keys);

#endif  // KEY_SET_H
