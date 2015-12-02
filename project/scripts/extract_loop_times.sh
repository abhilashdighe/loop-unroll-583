#!/bin/bash

# NOTE:
# ******************************************
# 1) LLVM_HOME = PATH TO LLVM HOME DIR
#    Set and export these environment variables in your /etc/profile OR /etc/bash.rc files

train_times_file=$1
fname=$2
input=$3
unroll_factor=$4
project_name="loop_unroll"
PASS_HOME="../$project_name"

clang -emit-llvm -o $fname.bc -c $fname.c || { echo "Failed to emit llvm bc"; exit 1; }

clang -emit-llvm -o timer.bc -c $PASS_HOME/lib/$project_name/timerFuncs.cpp || { echo "Failed to emit llvm bc for timers"; exit 1; }

llvm-link timer.bc $fname.bc -S -o=$fname.link.bc

echo "simplifying loop"
opt -loop-simplify < $fname.link.bc > $fname.ls.link.bc || { echo "Failed to opt loop simplify"; exit 1; }

echo "rotating loop"
opt  -mem2reg -loop-rotate < $fname.ls.link.bc > $fname.rotate.ls.link.bc  || { echo "Failed to opt loop rotate and mem2reg"; exit 1; }

echo "unrolling loop"
opt -load $PASS_HOME/Release+Asserts/lib/$project_name.so -benchmark $fname -debug -custom-unroll -custom-count $unroll_factor < $fname.simplify.ls.link.bc > $fname.unroll.rotate.ls.link.bc || { echo "Failed to unroll loop"; exit 1; }

echo "instrumenting loop"
opt -load $PASS_HOME/Release+Asserts/lib/$project_name.so -benchmark $fname -output-filename $train_times_file -loop-timer < $fname.unroll.rotate.ls.link.bc > $fname.li.unroll.rotate.ls.link.bc || { echo "Failed to instrument loops"; exit 1; }

llc $fname.li.unroll.rotate.ls.link.bc -o $fname.li.unroll.rotate.ls.link.s
g++ -o $fname.li.unroll.rotate.ls.link $fname.li.unroll.rotate.ls.link.bc
./$fname.li.unroll.rotate.ls.link