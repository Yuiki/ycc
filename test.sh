#!/bin/sh

set -e

echo 'COMPILE'
./build/ycc_gen1 ./test/test.c > ./build/test.s
echo 'LINK'
cc -o ./build/test ./build/test.s
echo 'RUN'
./build/test

echo 'OK'
