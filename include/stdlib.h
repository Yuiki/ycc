#ifndef _YCC_STDLIB
#define _YCC_STDLIB 1

extern void *calloc(int len, int size);
extern void exit(int status);

extern int strtol(char *nptr, char **endptr, int base);

#endif
