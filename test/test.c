int foo() { return 0; }

int echo(int x) { return x; }

void bar(int a, int b, int c, int d, int e, int f) {
  printf("%d\n", a + b + c + d + e + f);
}

void print(char c) { printf("%c\n", c); }

int **alloc4(int **p, int a, int b, int c, int d) {
  int *t;
  t = malloc(4 * sizeof(0));
  t[0] = a;
  t[1] = b;
  t[2] = c;
  t[3] = d;
  *p = t;
  return p;
}

void assert(int actual, int expected) {
  if (actual != expected) {
    printf("%d expected, but got %d\n", expected, actual);
    exit(1);
  }
}

void arith() {
  printf("test: arith\n");

  assert(0, 0);
  assert(42, 42);
  assert(5 + 20 - 4, 21);
  assert(12 + 34 - 5, 41);
  assert(5 + 6 * 7, 47);
  assert(5 * (9 - 6), 15);
  assert((3 + 5) / 2, 4);
  assert(-10 + 20, 10);
  assert(+5 - 4, 1);
}

void cmp() {
  printf("test: cmp\n");

  assert(1 == 1, 1);
  assert(1 == 2, 0);
  assert(1 != 1, 0);
  assert(1 != 2, 1);
  assert(2 > 1, 1);
  assert(10 > 20, 0);
  assert(2 >= 1, 1);
  assert(2 >= 2, 1);
  assert(10 >= 20, 0);
  assert(2 < 1, 0);
  assert(10 < 20, 1);
  assert(2 <= 1, 0);
  assert(2 <= 2, 1);
  assert(10 <= 20, 1);
}

void stmt() {
  printf("test: stmt\n");

  1 + 1;
  2 + 2;
  return;
}

void var1() {
  int a;
  a = 20;
  assert(a, 20);
}

void var2() {
  int a;
  int b;
  a = 20;
  b = 15;
  assert(a + b, 35);
}

void var3() {
  int foo;
  int bar;
  foo = 20;
  bar = 15;
  assert(foo + bar, 35);
}

void var4() {
  int a;
  int b;
  a = 3;
  b = 5 * 6 - 8;
  assert(a + b / 2, 14);
}

void vars() {
  printf("test: vars\n");

  var1();
  var2();
  var3();
  var4();
}

void if1() {
  if (1)
    return;
  else
    exit(1);
}

void if2() {
  int a;
  a = 0;
  if (a)
    exit(1);
  else
    return;
}

void if3() {
  int a;
  int b;
  a = 0;
  b = 1;
  if (a)
    exit(1);
  else if (b)
    return;
  else
    exit(1);
}

void ifs() {
  printf("test: ifs\n");

  if1();
  if2();
  if3();
}

void while1() {
  int i;
  i = 0;
  while (i < 10)
    i = i + 1;
  assert(i, 10);
}

void whiles() {
  printf("test: whiles\n");

  while1();
}

void for1() {
  int a;
  int i;
  a = 0;
  for (i = 1; i <= 10; i = i + 1)
    a = a + i;
  assert(a, 55);
}

void for2() {
  int a;
  int b;
  int i;
  a = 0;
  b = 0;
  for (i = 1; i <= 10; i = i + 1) {
    a = a + i;
    b = b + i * 2;
  }
  assert(a + b, 165);
}

void fors() {
  printf("test: fors\n");

  for1();
  for2();
}

void call1() { foo(); }

void call2() { bar(1, 2, 3, 4, 5, 6); }

int echo1(int x) { return x; }

void call3() { assert(echo1(100), 100); }

int echo2(int x) {
  int c;
  c = 3;
  return x;
}

void call4() {
  int a;
  int b;
  a = 1;
  b = 2;
  assert(echo2(100), 100);
}

int fib(int n) {
  if (n == 0)
    return 0;
  else if (n == 1)
    return 1;
  else
    return fib(n - 2) + fib(n - 1);
}

void call5() { assert(fib(11), 89); }

void calls() {
  printf("test: calls\n");

  call1();
  call2();
  call3();
  call4();
  call5();
}

void ref1() {
  int x;
  int *y;
  x = 3;
  y = &x;
  assert(*y, 3);
}

void ref2() {
  int x;
  int y;
  int *z;
  x = 3;
  y = 5;
  z = &y + 4;
  assert(*z, 3);
}

int *echo3(int *x) { return x; }

void ref3() {
  int x;
  int *y;
  x = 1;
  y = echo3(&x);
  assert(*y, 1);
}

void refs() {
  printf("test: refs\n");

  ref1();
  ref2();
  ref3();
}

int ptr1() {
  int *p;
  alloc4(&p, 1, 2, 4, 8);
  int *q;
  q = p + 2;
  assert(*q, 4);
}

void ptr2() {
  int *p;
  alloc4(&p, 1, 2, 4, 8);
  int *q;
  q = 2 + p;
  assert(*q, 4);
}

void ptr3() {
  int *p;
  alloc4(&p, 1, 2, 4, 8);
  int *q;
  q = p + 3;
  assert(*q, 8);
}

void ptr4() {
  int *p;
  alloc4(&p, 1, 2, 4, 8);
  int *q;
  q = p + 3;
  assert(*(q - 2), 2);
}

void ptrs() {
  printf("test: ptrs\n");

  ptr1();
  ptr2();
  ptr3();
  ptr4();
}

void sizeof1() {
  int x;
  assert(sizeof(x), 4);
}

void sizeof2() {
  int *y;
  assert(sizeof(y), 8);
}

void sizeof3() {
  int x;
  assert(sizeof(x + 3), 4);
}

void sizeof4() {
  int *y;
  assert(sizeof(y + 3), 8);
}

void sizeof5() {
  int *y;
  assert(sizeof(*y), 4);
}

void sizeof6() { assert(sizeof(1), 4); }

void sizeof7() { assert(sizeof(sizeof(1)), 4); }

void sizeofs() {
  printf("test: sizeofs\n");

  sizeof1();
  sizeof2();
  sizeof3();
  sizeof4();
  sizeof5();
  sizeof6();
  sizeof7();
}

void arr1() { int a[10]; }

void arr2() {
  int a[2];
  *a = 1;
  *(a + 1) = 2;
  int *p;
  p = a;
  assert(*p + *(p + 1), 3);
}

void arr3() {
  int a[1];
  a[0] = 1;
  assert(a[0], 1);
}

void arrs() {
  printf("test: arrs\n");

  arr1();
  arr2();
  arr3();
}

int global_x;
int global_y[20];
int *global_z;

int global() {
  printf("test: global\n");

  int z;
  global_x = 1;
  global_y[5] = 2;
  z = 3;
  assert(global_x + global_y[5] + z, 6);
  return 0;
}

void char1() {
  char x[3];
  x[0] = -1;
  x[1] = 2;
  int y;
  y = 4;
  assert(x[0] + y, 3);
}

void char2() {
  char x[1];
  x[0] = 65;
  print(x[0]);
}

void chars() {
  printf("test: chars\n");

  char1();
  char2();
}

void strlit() {
  printf("test: strlit\n");

  char *s;
  s = "Hello, world!";
  printf("%s\n", s);
  assert(s[0], 72);
}

// comment1
/* comment2
  multiline!
 */
void comment() {
  printf("test: comment\n");

  // comment3
  /* comment4
   */
  assert(3, 1 /* comment5 */ + 2); // comment6
}

void init_lvar1() {
  int x = 3;
  assert(x, 3);
}

void init_lvar2() {
  char arr[3];
  int len = sizeof(arr) / sizeof(arr[0]) + echo(0);
  assert(len, 3);
}

void init_lvar3() {
  char *msg1 = "foo";
  printf("%s\n", msg1);
}

void init_lvar4() {
  int y[5] = {1, 2, echo(3)};
  assert(y[0], 1);
  assert(y[1], 2);
  assert(y[2], 3);
  assert(y[3], 0);
  assert(y[4], 0);
}

void init_lvar5() { int x[] = {0, 1, 2}; }

void init_lvars() {
  printf("test: init_lvars\n");

  init_lvar1();
  init_lvar2();
  init_lvar3();
  init_lvar4();
  init_lvar5();
}

void inc1() {
  int i = 0;
  assert(i++, 0);
  assert(i, 1);
}

void incs() {
  printf("test: incs\n");

  inc1();
}

void dec1() {
  int i = 0;
  assert(i--, 0);
  assert(i, -1);
}

void decs() {
  printf("test: decs\n");

  dec1();
}

void nots() {
  printf("test: nots\n");

  assert(1, !0);
  assert(0, !!0);
}

void assign_eq() {
  int i = 2;
  i += 3;
  assert(i, 5);
}

void assigns() {
  printf("test: assigns\n");

  assign_eq();
}

int main() {
  arith();
  cmp();
  stmt();
  vars();
  ifs();
  whiles();
  fors();
  calls();
  refs();
  ptrs();
  sizeofs();
  arrs();
  global();
  chars();
  strlit();
  comment();
  init_lvars();
  incs();
  decs();
  nots();
  assigns();
  return 0;
}
