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

echo "profiling trip count"
rm temp_trip_count.csv
opt -analyze -mem2reg -indvars -scalar-evolution -load $PASS_HOME/Release+Asserts/lib/$project_name.so -tripCountExtractor -benchmark $fname -reuse-filename temp_trip_count.csv < $fname.ls.bc > $fname.mem2reg.ls.bc || { echo "Failed to get array feature stats"; exit 1; }

rm -r $1
echo "extracting features"
echo "benchmark, unique_loop_id, loop_nest_level, dynOp_count, floatOp_count, branchOp_count, memOp_count, operands_count, implicit_instr_count, unique_pred_count, trip_count, loop_calls, reuse_count, use_count, def_count, store_count, load_count" >> $1
opt -load $PASS_HOME/Release+Asserts/lib/$project_name.so -profile-loader -profile-info-file=llvmprof.out -featsExtractor -benchmark $fname -trip-count-filename temp_trip_count.csv -output-filename $train_file < $fname.ls.bc > $fname.ls.feats.bc || { echo "Failed to get feature stats"; exit 1; }
