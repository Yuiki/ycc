#!/bin/sh

set -e

echo 'COMPILE'
./build/ycc ./test/test.c > ./build/tmp.s
echo 'LINK'
cc -o ./build/tmp ./build/tmp.s
echo 'RUN'
./build/tmp

echo 'OK'
