#include "ycc.h"
#include <stdio.h>

char *filename;

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "Incorrect number of arguments\n");
    return 1;
  }

  filename = argv[1];
  user_input = read_file(filename);

  user_input = preprocess(user_input);

  token = tokenize(user_input);

  // parse
  program();

  // codegen
  gen_program();

  return 0;
}
