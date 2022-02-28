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

assert 0  "main(){0;}"
assert 42 "main(){42;}"
assert 21 "main(){5+20-4;}"
assert 41 "main(){ 12 + 34 - 5 ;}"
assert 47 "main(){5+6*7;}"
assert 15 "main(){5*(9-6);}"
assert 4  "main(){(3+5)/2;}"
assert 10 "main(){-10+20;}"
assert 1  "main(){+5-4;}"
assert 1 "main(){1==1;}"
assert 0 "main(){1==2;}"
assert 0 "main(){1!=1;}"
assert 1 "main(){1!=2;}"
assert 1 "main(){2>1;}"
assert 0 "main(){10>20;}"
assert 1 "main(){2>=1;}"
assert 1 "main(){2>=2;}"
assert 0 "main(){10>=20;}"
assert 0 "main(){2<1;}"
assert 1 "main(){10<20;}"
assert 0 "main(){2<=1;}"
assert 1 "main(){2<=2;}"
assert 1 "main(){10<=20;}"
assert 4 "main(){1+1;2+2;}"
assert 20 "main(){a=20;a;}"
assert 35 "main(){a=20;b=15;a+b;}"
assert 35 "main(){foo=20;bar=15;foo+bar;}"
assert 14 "main(){a = 3;b = 5 * 6 - 8;return a + b / 2;}"
assert 1  "main(){if (1) 1; else 0;}"
assert 22 "main(){a=0;if (a) 11; else 22;}"
assert 2  "main(){a=0;b=1;if (a) 1; else if (b) 2; else 3;}"
assert 10 "main(){i = 0; while (i < 10) i = i + 1; i;}"
assert 55 "main(){a = 0; for (i = 1; i <= 10; i = i + 1) a = a + i; a;}"
assert 165 "main(){a = 0; b = 0; for (i = 1; i <= 10; i = i + 1) { a = a + i; b = b + i * 2; } a + b;}"
assert 1  "main(){foo(); 1;}" "./build/test.o"
assert 1  "main(){bar(1, 2, 3, 4, 5, 6); 1;}" "./build/test.o"
assert 100 "main(){echo(100);}" "./build/test.o"
assert 100 "echo(x) { return x; } main(){echo(100);}"
assert 100 "echo(x) { c=3;return x; } main(){a=1;b=2;echo(100);}"
assert 89 "fib(n) { if (n == 0) return 0; else if (n == 1) return 1; else return fib(n - 2) + fib(n - 1); } main() { fib(11); }"
assert 3  "main() { x = 3; y = &x; return *y; }"
assert 3  "main() { x = 3; y = 5; z = &y + 8; return *z; }"

echo OK
