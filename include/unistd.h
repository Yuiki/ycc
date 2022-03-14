#include <stddef.h>

#ifndef _YCC_UNISTD
#define _YCC_UNISTD 1

typedef int pid_t;

extern int pipe(int *dest);

extern pid_t fork(void);

extern int close(int fd);

extern int dup2(int fd1, int fd2);

extern int execl(char *path, char *arg, ...);

extern int write(int fd, void *buf, int n);

extern int read(int fd, void *buf, int n);

#define STDIN_FILENO 0
#define STDOUT_FILENO 1

#endif
