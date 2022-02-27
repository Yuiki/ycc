#!/bin/sh
docker run --rm -it -v $(pwd):/cb -w /cb compilerbook gdb -ex run --args ./build/ycc "$1"
