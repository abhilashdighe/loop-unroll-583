#!/bin/bash

# NOTE:
# ******************************************
# 1) LLVM_HOME = PATH TO LLVM HOME DIR
#    Set and export these environment variables in your /etc/profile OR /etc/bash.rc files

train_times_file=$1
fname=$2
#input=$3
unroll_factor=$3
#unroll_factor=2
project_name="loop_unroll"
PASS_HOME="../$project_name"

rm .*dot
rm loopTimings.csv
clang -emit-llvm -o $fname.bc -c $fname.c || { echo "Failed to emit llvm bc"; exit 1; }

echo "creating cfg of test"
opt -dot-cfg $fname.bc >& /dev/null
mv cfg.main.dot pre.li.main.dot

clang -std=c++11 -emit-llvm -o timer.bc -c ../utils/timerFuncs.cpp || { echo "Failed to emit llvm bc for timers"; exit 1; }

echo "simplifying and rotating loop"
opt  -loop-simplify -mem2reg -loop-rotate < $fname.bc > $fname.rotate.ls.link.bc  || { echo "Failed to opt loop-simplify loop rotate and mem2reg"; exit 1; }

echo "instrumenting loop"
opt -load $PASS_HOME/Release+Asserts/lib/$project_name.so -benchmark $fname -module-inst -loop-inst < $fname.rotate.ls.link.bc > $fname.li.unroll.rotate.ls.link.bc || { echo "Failed to instrument loops"; exit 1; }

echo "unrolling loop"
opt -load $PASS_HOME/Release+Asserts/lib/$project_name.so -benchmark $fname -debug -custom-unroll -custom-count $unroll_factor < $fname.li.unroll.rotate.ls.link.bc > $fname.unroll.rotate.ls.link.bc || { echo "Failed to unroll loop"; exit 1; }


echo "Executing unrolled and instrumented benchmark"

llvm-link timer.bc $fname.unroll.rotate.ls.link.bc -S -o=$fname.op
lli $fname.op

echo "creating cfg of instrumented test"
opt -dot-cfg $fname.unroll.rotate.ls.link.bc >& /dev/null
mv cfg.main.dot post.li.main.dot
#mv *.dot ../tests/

#mv loopTimings.csv ../tests/
