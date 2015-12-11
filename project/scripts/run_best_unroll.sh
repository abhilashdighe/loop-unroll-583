#!/bin/bash

# NOTE:
# ******************************************
# 1) LLVM_HOME = PATH TO LLVM HOME DIR
#    Set and export these environment variables in your /etc/profile OR /etc/bash.rc files

unroll_factors_file="factors.csv"
# Replace with bench
fname=$1
project_name="loop_unroll"
PASS_HOME="../$project_name"

clang -emit-llvm -o $fname.bc -c $fname.c || { echo "Failed to emit llvm bc"; exit 1; }

rm pre.main.dot
rm post.main.dot

echo "Making CFG of test file"
opt -dot-cfg $fname.bc
mv cfg.main.dot pre.main.dot

echo "simplifying and rotating loop"
opt  -loop-simplify -mem2reg -loop-rotate < $fname.bc > $fname.rotate.ls.link.bc  || { echo "Failed to opt loop-simplify loop rotate and mem2reg"; exit 1; }

echo "unrolling loop"
opt -load $PASS_HOME/Release+Asserts/lib/$project_name.so -best-unroll -factors-file $unroll_factors_file -benchmark $fname -debug  < $fname.rotate.ls.link.bc > $fname.best.unroll.bc || { echo "Failed to unroll loop"; exit 1; }

echo "Making CFG of unrolled file"
opt -dot-cfg $fname.best.unroll.bc
mv cfg.main.dot post.main.dot

echo "Executing unrolled and instrumented benchmark"
lli $fname.best.unroll.bc
