#!/bin/sh
assert() {
    expected="$1"
    input="$2"
    link_file="$3"

    ./build/ycc "$input" > ./build/tmp.s
    cc -o ./build/tmp ./build/tmp.s $link_file
    ./build/tmp
    actual="$?"

    if [ "$actual" = "$expected" ]; then
        echo "$input => $actual"
    else
        echo "$input => $expected expected, but got $actual"
        exit 1
    fi
}

assert 0  "int main(){return 0;}"
assert 42 "int main(){return 42;}"
assert 21 "int main(){return 5+20-4;}"
assert 41 "int main(){return 12 + 34 - 5 ;}"
assert 47 "int main(){return 5+6*7;}"
assert 15 "int main(){return 5*(9-6);}"
assert 4  "int main(){return (3+5)/2;}"
assert 10 "int main(){return -10+20;}"
assert 1  "int main(){return +5-4;}"
assert 1 "int main(){return 1==1;}"
assert 0 "int main(){return 1==2;}"
assert 0 "int main(){return 1!=1;}"
assert 1 "int main(){return 1!=2;}"
assert 1 "int main(){return 2>1;}"
assert 0 "int main(){return 10>20;}"
assert 1 "int main(){return 2>=1;}"
assert 1 "int main(){return 2>=2;}"
assert 0 "int main(){return 10>=20;}"
assert 0 "int main(){return 2<1;}"
assert 1 "int main(){return 10<20;}"
assert 0 "int main(){return 2<=1;}"
assert 1 "int main(){return 2<=2;}"
assert 1 "int main(){return 10<=20;}"
assert 4 "int main(){1+1; return 2+2;}"
assert 20 "int main(){int a;a=20;return a;}"
assert 35 "int main(){int a;int b;a=20;b=15;return a+b;}"
assert 35 "int main(){int foo;int bar;foo=20;bar=15;return foo+bar;}"
assert 14 "int main(){int a;int b;a = 3;b = 5 * 6 - 8;return a + b / 2;}"
assert 1  "int main(){if (1) return 1; else return 0;}"
assert 22 "int main(){int a;a=0;if (a) return 11; else return 22;}"
assert 2  "int main(){int a;int b;a=0;b=1;if (a) return 1; else if (b) return 2; else return 3;}"
assert 10 "int main(){int i; i = 0; while (i < 10) i = i + 1; return i;}"
assert 55 "int main(){int a; int i; a = 0; for (i = 1; i <= 10; i = i + 1) a = a + i; return a;}"
assert 165 "int main(){int a; int b; int i; a = 0; b = 0; for (i = 1; i <= 10; i = i + 1) { a = a + i; b = b + i * 2; } return a + b;}"
assert 1  "int main(){foo(); return 1;}" "./build/test.o"
assert 1  "int main(){bar(1, 2, 3, 4, 5, 6); return 1;}" "./build/test.o"
assert 100 "int main(){return echo(100);}" "./build/test.o"
assert 100 "int echo(int x) { return x; } int main(){echo(100);}"
assert 100 "int echo(int x) { int c;c=3;return x; } int main(){int a;int b;a=1;b=2;echo(100);}"
assert 89 "int fib(int n) { if (n == 0) return 0; else if (n == 1) return 1; else return fib(n - 2) + fib(n - 1); } int main() { fib(11); }"
assert 3  "int main() { int x; int y; x = 3; y = &x; return *y; }"
assert 3  "int main() { int x; int y; int z; x = 3; y = 5; z = &y + 8; return *z; }"
assert 3  "int main(){ int x; int *y; y = &x; *y = 3; return x;}"
assert 1  "int *echo(int *x) { return x; } int main(){ int x; int *y; x = 1; y = echo(x); return y; }"
assert 4 "int main(){int *p; alloc4(&p, 1, 2, 4, 8); int *q;q = p + 2; return *q;}" "./build/test.o"
assert 4 "int main(){int *p; alloc4(&p, 1, 2, 4, 8); int *q;q = 2 + p; return *q;}" "./build/test.o"
assert 8 "int main(){int *p; alloc4(&p, 1, 2, 4, 8); int *q;q = p + 3; return *q;}" "./build/test.o"
assert 2 "int main(){int *p; alloc4(&p, 1, 2, 4, 8); int *q;q = p + 3; return *(q - 2);}" "./build/test.o"
assert 4  "int main() { int x; return sizeof(x); }"
assert 8  "int main() { int *y; return sizeof(y); }"
assert 4  "int main() { int x; return sizeof(x + 3); }"
assert 8  "int main() { int *y; return sizeof(y + 3); }"
assert 4  "int main() { int *y; return sizeof(*y); }"
assert 4  "int main() { return sizeof(1); }"
assert 4  "int main() { return sizeof(sizeof(1)); }"
assert 1  "int main() { int a[10]; return 1; }"
assert 3  "int main(){ int a[2]; *a = 1; *(a + 1) = 2; int *p; p = a; return *p + *(p + 1); }"
assert 1  "int main() { int a[1]; a[0] = 1; return a[0]; }"
assert 6  "int x; int y[20]; int *z; int main() { int z; x = 1; y[5] = 2; z = 3; return x + y[5] + z; }"
assert 3  "int main(){ char x[3]; x[0] = -1; x[1] = 2; int y; y = 4; return x[0] + y; }"
assert 0  "int main(){ char x[1]; x[0] = 128; print(x[0]); return 0; }" "./build/test.o"
assert 72 '
int main() {
    char *s;
    s = "Hello, world!";
    printf("%s", s);
    return s[0];
}'

echo OK
