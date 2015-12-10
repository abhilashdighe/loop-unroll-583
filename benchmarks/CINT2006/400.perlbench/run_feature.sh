#!/bin/bash
make clean
make TEST=simplify
train_file="train.txt"
project_name="loop_unroll"
PASS_HOME="/home/vgogte/eecs583/loop-unroll-583/project/loop_unroll"

fname="400.perlbench"
PASS_HOME="/home/vgogte/eecs583/loop-unroll-583/project/loop_unroll"
INPUTFILE="/home/vgogte/spec2006/benchspec/CPU2006/400.perlbench/data/all/input/diffmail.pl"
OUTPUTFILE=""
STDOUTFILE="perlbench.all.diffmail.out"
STDERRFILE="dperlbench.ref.diffmail.err"						
ARGUMENTS=" -I/home/vgogte/spec2006/benchspec/CPU2006/400.perlbench/data/all/input/lib $INPUTFILE 4 800 10 17 19 300"

PATH=$PATH:/home/vgogte/build/Release+Asserts/bin
cd Output
rm temp_array_reuse.csv
opt -insert-edge-profiling $fname.linked.rbc -o $fname.profile.ls.bc
llc $fname.profile.ls.bc -o $fname.profile.ls.s
g++ -o $fname.profile $fname.profile.ls.s /home/vgogte/build/Release+Asserts/lib/libprofile_rt.so
./$fname.profile $ARGUMENTS 

echo "profiling trip count"
rm temp_trip_count.csv
opt -analyze -mem2reg -indvars -scalar-evolution -load $PASS_HOME/Release+Asserts/lib/$project_name.so -tripCountExtractor -benchmark $fname -reuse-filename temp_trip_count.csv < $fname.linked.rbc > $fname.mem2reg.ls.bc || { echo "Failed to get array feature stats"; exit 1; }

echo "extracting features"
echo "benchmark, unique_loop_id, loop_nest_level, dynOp_count, floatOp_count, branchOp_count, memOp_count, operands_count, implicit_instr_count, unique_pred_count, trip_count, loop_calls, reuse_count, use_count, def_count, store_count, load_count" >> $train_file 
opt -load $PASS_HOME/Release+Asserts/lib/$project_name.so -profile-loader -profile-info-file=llvmprof.out -featsExtractor -benchmark $fname -trip-count-filename temp_trip_count.csv -output-filename $train_file < $fname.linked.rbc > $fname.ls.feats.bc || { echo "Failed to get feature stats"; exit 1; }

cp $train_file ~/eecs583/loop-unroll-583/results/$fname\_train.txt
#llc $fname.linked.rbc -o $fname.s
#g++ $fname.s -o $fname.exe
#./$fname.exe $ARGUMENTS 
