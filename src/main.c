#include "ycc.h"
#include <stdio.h>

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "引数の個数が正しくありません\n");
    return 1;
  }

  filename = argv[1];
  user_input = read_file(filename);
  token = tokenize(user_input);
  program();

  printf(".intel_syntax noprefix\n");
  printf(".globl main\n");

  for (Str *str = strs; str; str = str->next) {
    printf(".LC%d:\n", str->index);
    printf("  .string %.*s\n", str->len, str->value);
  }

  for (int i = 0; globals[i]; i++) {
    gen(globals[i]);
  }

  return 0;
}
