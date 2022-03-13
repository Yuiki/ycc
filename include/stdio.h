#include <stdarg.h>

#ifndef _YCC_STDIO
#define _YCC_STDIO 1

typedef struct {
} FILE;

extern FILE *stderr;
#define stderr stderr

#define SEEK_SET 0
#define SEEK_END 2

extern int printf(char *fmt, ...);

extern int fprintf(FILE *s, char *fmt, ...);

extern int vfprintf(FILE *s, char *fmt, va_list arg);

extern FILE *fopen(char filename, char *modes);

extern int fseek(FILE *stream, int off, int whence);

extern int ftell(FILE *stream);

extern int fread(void *ptr, int size, int n, FILE *stream);

extern int fclose(FILE *stream);

#endif
