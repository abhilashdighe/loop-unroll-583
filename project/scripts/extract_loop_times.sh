#!/bin/bash

# NOTE:
# ******************************************
# 1) LLVM_HOME = PATH TO LLVM HOME DIR
#    Set and export these environment variables in your /etc/profile OR /etc/bash.rc files

train_times_file=$1
fname=$2
#input=$3
#unroll_factor=$4
unroll_factor=1
project_name="loop_unroll"
PASS_HOME="../$project_name"

clang -emit-llvm -o $fname.bc -c $fname.c || { echo "Failed to emit llvm bc"; exit 1; }

echo "creating cfg of test"
opt -dot-cfg $fname.bc >& /dev/null
mv cfg.main.dot pre.li.main.dot

clang -std=c++11 -emit-llvm -o timer.bc -c $PASS_HOME/lib/$project_name/timerFuncs.cpp || { echo "Failed to emit llvm bc for timers"; exit 1; }

echo "simplifying loop"
opt -loop-simplify < $fname.bc > $fname.ls.link.bc || { echo "Failed to opt loop simplify"; exit 1; }

echo "rotating loop"
opt  -mem2reg -loop-rotate < $fname.ls.link.bc > $fname.rotate.ls.link.bc  || { echo "Failed to opt loop rotate and mem2reg"; exit 1; }

#echo "unrolling loop"
#opt -load $PASS_HOME/Release+Asserts/lib/$project_name.so -benchmark $fname -debug -custom-unroll -custom-count $unroll_factor < $fname.rotate.ls.link.bc > $fname.unroll.rotate.ls.link.bc || { echo "Failed to unroll loop"; exit 1; }

echo "instrumenting loop"
opt -load $PASS_HOME/Release+Asserts/lib/$project_name.so -benchmark $fname -module-inst -loop-inst < $fname.ls.link.bc > $fname.li.unroll.rotate.ls.link.bc || { echo "Failed to instrument loops"; exit 1; }

echo "Executing unrolled and instrumented benchmark"

llvm-link timer.bc $fname.li.unroll.rotate.ls.link.bc -S -o=$fname.op
lli $fname.op

echo "creating cfg of instrumented test"
opt -dot-cfg $fname.li.unroll.rotate.ls.link.bc >& /dev/null
mv cfg.main.dot post.li.main.dot
#mv *.dot ../tests/

#mv loopTimings.csv ../tests/
