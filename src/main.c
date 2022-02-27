#include "ycc.h"
#include <stdio.h>

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "引数の個数が正しくありません\n");
    return 1;
  }

  user_input = argv[1];
  token = tokenize(user_input);
  program();

  printf(".intel_syntax noprefix\n");
  printf(".globl main\n");

  for (int i = 0; functions[i]; i++) {
    gen(functions[i]);
  }

  return 0;
}
