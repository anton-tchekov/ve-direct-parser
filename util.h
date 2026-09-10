#ifndef __UTIL_H__
#define __UTIL_H__

#define ARRLEN(X) (sizeof(X) / sizeof(*X))

int starts_with(const char *s, const char *prefix);
char *copy_str(char *out, int len, const char *v);
int nibble_to_ascii(int v);

#endif
