#!/bin/bash

# NOTE:
# ******************************************
# 1) LLVM_HOME = PATH TO LLVM HOME DIR
#    Set and export these environment variables in your /etc/profile OR /etc/bash.rc files

train_file=$1
fname=$2
input=$3
project_name="loop_unroll"
PASS_HOME="../$project_name"

echo $PASS_HOME

rm llvmprof.out
clang -emit-llvm -o $fname.bc -c $fname.c || { echo "Failed to emit llvm bc"; exit 1; }

echo "simplifying loop"
opt -loop-simplify < $fname.bc > $fname.ls.bc || { echo "Failed to opt loop simplify"; exit 1; }

echo "edge profiling"
opt -insert-edge-profiling $fname.ls.bc -o $fname.profile.ls.bc
llc $fname.profile.ls.bc -o $fname.profile.ls.s
g++ -o $fname.profile $fname.profile.ls.s $LLVM_HOME/Release+Asserts/lib/libprofile_rt.so
./$fname.profile $input> /dev/null

echo "lamp profiling"
opt -load $PASS_HOME/Release+Asserts/lib/$project_name.so -lamp-insts -insert-lamp-profiling -insert-lamp-loop-profiling -insert-lamp-init < $fname.ls.bc > $fname.lamp.bc || { echo "Failed to do lamp profiling"; exit 1; }
llc < $fname.lamp.bc > $fname.lamp.s || { echo "Failed to llc"; exit 1; }
g++ -o $fname.lamp.exe $fname.lamp.s $PASS_HOME/tools/lamp-profiler/lamp_hooks.o || { echo "Failed to add lamp hooks"; exit 1; }
./$fname.lamp.exe $input> /dev/null

echo "extracting features"
opt -analyze -mem2reg -indvars -scalar-evolution -stats -load $PASS_HOME/Release+Asserts/lib/$project_name.so -lamp-inst-cnt -lamp-map-loop -lamp-load-profile -profile-loader -profile-info-file=llvmprof.out -featsExtractor -benchmark $fname -output-filename $train_file < $fname.ls.bc > $fname.ls.feats.bc || { echo "Failed to get feature stats"; exit 1; }
