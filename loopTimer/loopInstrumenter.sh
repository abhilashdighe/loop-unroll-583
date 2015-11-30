#!/bin/bash


source ~/.zshrc
fname=$1
input=$2

make clean -C $PASS_HOME
make -C $PASS_HOME
echo "PASS HOME PRINT"
echo $PASS_HOME

# Create input bc file
clang -emit-llvm -o $fname.bc -c $fname.c || { echo "Failed to emit llvm bc for input"; exit 1; }

# Create bc file for TimerFunc.cpp
clang -emit-llvm -o timer.bc -c $PASS_HOME/lib/timerFuncs.cpp || { echo "Failed to emit llvm bc for timers"; exit 1; }

# Link TimerFuncs.cpp and input file together
llvm-link timer.bc $fname.bc -S -o=pass.bc

# Create cfg of test input file for reference
opt -dot-cfg $fname.bc >& /dev/null
mv cfg.main.dot pre.main.dot

opt -load $PASS_HOME/Release+Asserts/lib/loopTimer.so -loop-timer < pass.bc > $fname.li.bc || { echo "Failed to instrument loops"; exit 1; }
