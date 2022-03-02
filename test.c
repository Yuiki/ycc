#include <stdio.h>
#include <stdlib.h>

void foo() { printf("OK\n"); }

void bar(int a, int b, int c, int d, int e, int f) {
  printf("%d\n", a + b + c + d + e + f);
}

int echo(int x) { return x; }

int **alloc4(int **p, int a, int b, int c, int d) {
  *p = malloc(4 * sizeof(int));
  (*p)[0] = a;
  (*p)[1] = b;
  (*p)[2] = c;
  (*p)[3] = d;
  return p;
}

void print(int c) { printf("%d\n", c); }
