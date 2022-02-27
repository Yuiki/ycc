#!/bin/sh
assert() {
    expected="$1"
    input="$2"

    ./build/ycc "$input" > ./build/tmp.s
    cc -o ./build/tmp ./build/tmp.s
    ./build/tmp
    actual="$?"
      
    if [ "$actual" = "$expected" ]; then
        echo "$input => $actual"
    else
        echo "$input => $expected expected, but got $actual"
        exit 1
    fi
}

assert 0  "0;"
assert 42 "42;"
assert 21 "5+20-4;"
assert 41 " 12 + 34 - 5 ;"
assert 47 "5+6*7;"
assert 15 "5*(9-6);"
assert 4  "(3+5)/2;"
assert 10 "-10+20;"
assert 1  "+5-4;"
assert 1 "1==1;"
assert 0 "1==2;"
assert 0 "1!=1;"
assert 1 "1!=2;"
assert 1 "2>1;"
assert 0 "10>20;"
assert 1 "2>=1;"
assert 1 "2>=2;"
assert 0 "10>=20;"
assert 0 "2<1;"
assert 1 "10<20;"
assert 0 "2<=1;"
assert 1 "2<=2;"
assert 1 "10<=20;"
assert 4 "1+1;2+2;"
assert 20 "a=20;a;"
assert 35 "a=20;b=15;a+b;"
assert 35 "foo=20;bar=15;foo+bar;"
assert 14 "a = 3;b = 5 * 6 - 8;return a + b / 2;"
assert 1  "if (1) 1; else 0;"
assert 22 "a=0;if (a) 11; else 22;"
assert 2  "a=0;b=1;if (a) 1; else if (b) 2; else 3;"
assert 10 "i = 0; while (i < 10) i = i + 1; i;"

echo OK
