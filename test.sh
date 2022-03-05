#!/bin/sh

./build/ycc ./test/test.c > ./build/tmp.s
cc -o ./build/tmp ./build/tmp.s
./build/tmp

if [ $? -eq 0 ]; then
  echo 'OK'
else
  echo 'NG'
fi
