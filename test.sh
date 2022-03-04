#!/bin/sh

./build/ycc ./test/test.c > ./build/tmp.s
cc -o ./build/tmp ./build/tmp.s
./build/tmp

echo '\nOK'
