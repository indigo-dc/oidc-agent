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

void  _secFreeKeySetSEstr(struct keySetSEstr keys);
void  _secFreeKeySetPPstr(struct keySetPPstr keys);
char* keySetSEToJSONString(const struct keySetSEstr keys);

#ifndef secFreeKeySetSEstr
#define secFreeKeySetSEstr(s) \
  do {                        \
    _secFreeKeySetSEstr((s)); \
    (s).sign = NULL;          \
    (s).enc  = NULL;          \
  } while (0)
#endif  // secFreeKeySetSEstr

#ifndef secFreeKeySetPPstr
#define secFreeKeySetPPstr(s) \
  do {                        \
    _secFreeKeySetPPstr((s)); \
    (s).priv = NULL;          \
    (s).pub  = NULL;          \
  } while (0)
#endif  // secFreeKeySetPPstr

#endif  // KEY_SET_H
