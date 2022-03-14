#!/bin/sh
docker run --rm -it -v $(pwd):/cb -w /cb compilerbook bash -c "make && gdb -ex run --args ./build/gen1/ycc $1"
