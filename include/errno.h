#ifndef _YCC_ERRNO
#define _YCC_ERRNO 1

extern int *__errno_location(void);

#define errno (*__errno_location())

#endif
