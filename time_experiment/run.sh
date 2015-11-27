#!/bin/sh
clang -emit-llvm -o hello.bc -c hello_world.c
opt -dot-cfg hello.bc > /dev/null