#include "ycc.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

char *filename;

// Report error and exit
// the args are same as printf
void error(char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

// Report error and its position and exit
// the args are same as printf
void error_at(char *loc, char *fmt, ...) {
  char *line = loc;
  while (user_input < line && line[-1] != '\n') {
    line--;
  }

  char *end = loc;
  while (*end != '\n') {
    end++;
  }

  int line_num = 1;
  for (char *p = user_input; p < line; p++) {
    if (*p == '\n') {
      line_num++;
    }
  }

  int indent = fprintf(stderr, "%s:%d ", filename, line_num);
  fprintf(stderr, "%.*s\n", (int)(end - line), line);

  va_list ap;
  va_start(ap, fmt);

  int pos = loc - line + indent;
  fprintf(stderr, "%*s", pos, " ");
  fprintf(stderr, "^ ");
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}
