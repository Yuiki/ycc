# ycc: x86-64 C Compiler (hobby) written in C

(Work in Progress)

This compiler is based on https://www.sigbus.info/compilerbook .

## Preparation

1. `$ docker build -t compilerbook https://www.sigbus.info/compilerbook/Dockerfile`

## How to use

1. `$ ./shell.sh`
2. `$ make`
3. `$ ./build/ycc <target C file path>` (output the assembly to stdout)

## How to test

1. `$ ./test-docker.sh`
