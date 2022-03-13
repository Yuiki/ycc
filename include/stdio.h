#ifndef _YCC_STDIO
#define _YCC_STDIO 1

typedef struct {
} FILE;

extern FILE *stderr;
#define stderr stderr

#define SEEK_SET 0
#define SEEK_END 2

#endif
