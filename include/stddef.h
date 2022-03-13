#define __attribute__(_)

#define __asm__(_)

// TODO: support
#define signed
#define unsigned
#define long
#define short int

#ifndef _YCC_STDDEF
#define _YCC_STDDEF 1

// TODO: use unsigned long
typedef int size_t;

// TODO: support properly
typedef short int int;

#endif
