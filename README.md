# ycc: x86-64 C Compiler written in C

ycc is a self-hosted x86-64 C Compiler written in C.

This is initially based on https://www.sigbus.info/compilerbook (compilerbook).  
I create this compiler for my study :)

## Disclaimer

This is partially compliant with C11 specification.  
The implementation is work in progress, so ycc is still a toy.

## Prerequisite

- Docker
- x86-64 Environment

## Preparation

1. `$ docker build -t compilerbook https://www.sigbus.info/compilerbook/Dockerfile`

## How to build

1. `$ ./shell.sh`
2. `$ make`

## Varieties

You can use 1st, 2nd and 3rd gen compilers.

| Path               | Gen | Description                                   |
| :----------------- | :-- | :-------------------------------------------- |
| `./build/ycc_gen1` | 1st | Compiled by `cc`                              |
| `./build/ycc_gen2` | 2nd | Compiled by `./build/ycc_gen1`                |
| `./build/ycc_gen3` | 3rd | Compiled by `./build/ycc_gen2` (Same as gen2) |

## How to use

1. `$ ycc <target C file path>` (output the assembly to stdout)

## How to test

1. `$ ./test-docker.sh`

## Difference from compilerbook

- Self-hosted
- Struct
- Enum
- Variable Arguments
- `short`, `_Bool` and `void` type
- `typedef`
- Function declaration
- `switch`
- Scope
- char literal
- `continue` and `break`
- `%`, `||`, `&&`, `+=`, `!`, `++` and `--` operators
- and so on!

## Todo

WIP
