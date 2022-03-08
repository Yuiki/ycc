int foo() { return 0; }

int echo(int x) { return x; }

int bar(int a, int b, int c, int d, int e, int f) {
  printf("%d\n", a + b + c + d + e + f);
  return 0;
}

int print(char c) {
  printf("%c\n", c);
  return 0;
}

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

int assert(int actual, int expected) {
  if (actual != expected) {
    printf("%d expected, but got %d\n", expected, actual);
    exit(1);
  }
  return 0;
}

int arith() {
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
  return 0;
}

int cmp() {
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
  return 0;
}

int stmt() {
  printf("test: stmt\n");

  1 + 1;
  return 2 + 2;
}

int var1() {
  int a;
  a = 20;
  assert(a, 20);
  return 0;
}

int var2() {
  int a;
  int b;
  a = 20;
  b = 15;
  assert(a + b, 35);
  return 0;
}

int var3() {
  int foo;
  int bar;
  foo = 20;
  bar = 15;
  assert(foo + bar, 35);
  return 0;
}

int var4() {
  int a;
  int b;
  a = 3;
  b = 5 * 6 - 8;
  assert(a + b / 2, 14);
  return 0;
}

int vars() {
  printf("test: vars\n");

  var1();
  var2();
  var3();
  var4();
  return 0;
}

int if1() {
  if (1)
    return 0;
  else
    exit(1);
}

int if2() {
  int a;
  a = 0;
  if (a)
    exit(1);
  else
    return 0;
}

int if3() {
  int a;
  int b;
  a = 0;
  b = 1;
  if (a)
    exit(1);
  else if (b)
    return 0;
  else
    exit(1);
}

int ifs() {
  printf("test: ifs\n");

  if1();
  if2();
  if3();
  return 0;
}

int while1() {
  int i;
  i = 0;
  while (i < 10)
    i = i + 1;
  assert(i, 10);
  return 0;
}

int whiles() {
  printf("test: whiles\n");

  while1();
  return 0;
}

int for1() {
  int a;
  int i;
  a = 0;
  for (i = 1; i <= 10; i = i + 1)
    a = a + i;
  assert(a, 55);
  return 0;
}

int for2() {
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
  return 0;
}

int fors() {
  printf("test: fors\n");

  for1();
  for2();
  return 0;
}

int call1() {
  foo();
  return 0;
}

int call2() {
  bar(1, 2, 3, 4, 5, 6);
  return 1;
}

int echo1(int x) { return x; }

int call3() {
  assert(echo1(100), 100);
  return 0;
}

int echo2(int x) {
  int c;
  c = 3;
  return x;
}

int call4() {
  int a;
  int b;
  a = 1;
  b = 2;
  assert(echo2(100), 100);
  return 0;
}

int fib(int n) {
  if (n == 0)
    return 0;
  else if (n == 1)
    return 1;
  else
    return fib(n - 2) + fib(n - 1);
}

int call5() {
  assert(fib(11), 89);
  return 0;
}

int calls() {
  printf("test: calls\n");

  call1();
  call2();
  call3();
  call4();
  call5();
  return 0;
}

int ref1() {
  int x;
  int *y;
  x = 3;
  y = &x;
  assert(*y, 3);
  return 0;
}

int ref2() {
  int x;
  int y;
  int *z;
  x = 3;
  y = 5;
  z = &y + 4;
  assert(*z, 3);
  return 0;
}

int *echo3(int *x) { return x; }

int ref3() {
  int x;
  int *y;
  x = 1;
  y = echo3(&x);
  assert(*y, 1);
  return 0;
}

int refs() {
  printf("test: refs\n");

  ref1();
  ref2();
  ref3();
  return 0;
}

int ptr1() {
  int *p;
  alloc4(&p, 1, 2, 4, 8);
  int *q;
  q = p + 2;
  assert(*q, 4);
  return 0;
}

int ptr2() {
  int *p;
  alloc4(&p, 1, 2, 4, 8);
  int *q;
  q = 2 + p;
  assert(*q, 4);
  return 0;
}

int ptr3() {
  int *p;
  alloc4(&p, 1, 2, 4, 8);
  int *q;
  q = p + 3;
  assert(*q, 8);
  return 0;
}

int ptr4() {
  int *p;
  alloc4(&p, 1, 2, 4, 8);
  int *q;
  q = p + 3;
  assert(*(q - 2), 2);
  return 0;
}

int ptrs() {
  printf("test: ptrs\n");

  ptr1();
  ptr2();
  ptr3();
  ptr4();
  return 0;
}

int sizeof1() {
  int x;
  assert(sizeof(x), 4);
  return 0;
}

int sizeof2() {
  int *y;
  assert(sizeof(y), 8);
  return 0;
}

int sizeof3() {
  int x;
  assert(sizeof(x + 3), 4);
  return 0;
}

int sizeof4() {
  int *y;
  assert(sizeof(y + 3), 8);
  return 0;
}

int sizeof5() {
  int *y;
  assert(sizeof(*y), 4);
  return 0;
}

int sizeof6() {
  assert(sizeof(1), 4);
  return 0;
}

int sizeof7() {
  assert(sizeof(sizeof(1)), 4);
  return 0;
}

int sizeofs() {
  printf("test: sizeofs\n");

  sizeof1();
  sizeof2();
  sizeof3();
  sizeof4();
  sizeof5();
  sizeof6();
  sizeof7();
  return 0;
}

int arr1() {
  int a[10];
  return 0;
}

int arr2() {
  int a[2];
  *a = 1;
  *(a + 1) = 2;
  int *p;
  p = a;
  assert(*p + *(p + 1), 3);
  return 0;
}

int arr3() {
  int a[1];
  a[0] = 1;
  assert(a[0], 1);
  return 0;
}

int arrs() {
  printf("test: arrs\n");

  arr1();
  arr2();
  arr3();
  return 0;
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
  return global_x + global_y[5] + z;
}

int char1() {
  char x[3];
  x[0] = -1;
  x[1] = 2;
  int y;
  y = 4;
  assert(x[0] + y, 3);
  return 0;
}

int char2() {
  char x[1];
  x[0] = 65;
  print(x[0]);
  return 0;
}

int chars() {
  printf("test: chars\n");

  char1();
  char2();
  return 0;
}

int strlit() {
  printf("test: strlit\n");

  char *s;
  s = "Hello, world!";
  printf("%s\n", s);
  assert(s[0], 72);
  return 0;
}

// comment1
/* comment2
  multiline!
 */
int comment() {
  // comment3
  /* comment4
   */
  assert(3, 1 /* comment5 */ + 2); // comment6
  return 0;
}

int init_lvar1() {
  int x = 3;
  assert(x, 3);
  return 0;
}

int init_lvar2() {
  char arr[3];
  int len = sizeof(arr) / sizeof(arr[0]) + echo(0);
  assert(len, 3);
  return 1;
}

int init_lvars() {
  init_lvar1();
  init_lvar2();
  return 0;
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
  return 0;
}
