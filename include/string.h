#ifndef _YCC_STRING
#define _YCC_STRING 1

extern char *strerror(int errnum);

extern int memcmp(void *s1, void *s2, int n);

extern int strlen(char *s);

extern void *memcpy(void *dest, void *src, int n);

extern int strncmp(char *s1, char *s2, int n);

extern char *strstr(char *big, char *little);

#endif
