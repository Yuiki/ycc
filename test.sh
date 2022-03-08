#!/bin/sh

echo 'COMPILE'
./build/ycc ./test/test.c > ./build/tmp.s
echo 'LINK'
cc -o ./build/tmp ./build/tmp.s
echo 'RUN'
./build/tmp

if [ $? -eq 0 ]; then
  echo 'OK'
else
  echo 'NG'
fi
