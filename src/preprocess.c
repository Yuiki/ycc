#include "ycc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

// TODO: replace to our own preprocessor
char *preprocess(char *program) {
  int pipe_parent[2];
  int pipe_child[2];
  pipe(pipe_parent);
  pipe(pipe_child);

  pid_t cpid = fork();

  if (cpid == 0) { // child
    close(pipe_parent[STDOUT_FILENO]);
    dup2(pipe_parent[STDIN_FILENO], STDIN_FILENO);
    dup2(pipe_child[STDOUT_FILENO], STDOUT_FILENO);

    execl("/usr/bin/cpp", "/usr/bin/cpp", "-I", "include", "-I", "src", "-P",
          NULL);

    error("not reachable");
  } else { // parent
    write(pipe_parent[STDOUT_FILENO], program, strlen(program));
    close(pipe_parent[STDOUT_FILENO]);

    // TODO: size
    int len = 1048576;
    char *buf = calloc(len, sizeof(char));
    wait(0);
    read(pipe_child[STDIN_FILENO], buf, len);
    return buf;
  }

  error("not reachable");
  return NULL;
}
