#define __attribute__(_)

#define __asm__(_)

// TODO: support
#define signed
#define unsigned
#define long
#define double int
#define float int
#define static
#define __inline
#define volatile

#ifndef _YCC_STDDEF
#define _YCC_STDDEF 1

// TODO: use ((void *)0)
#define NULL 0

#define const

#define __restrict

#define __extension__

// TODO: use unsigned long
typedef int size_t;

typedef char wchar_t;

#endif
