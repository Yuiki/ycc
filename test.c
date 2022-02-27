#include <stdio.h>

void foo() { printf("OK\n"); }

void bar(int a, int b, int c, int d, int e, int f) {
  printf("%d\n", a + b + c + d + e + f);
}

int echo(int x) { return x; }
