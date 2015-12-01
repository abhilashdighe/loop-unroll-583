#!/bin/bash
fname=$1
input=$2

make clean -C /home/funke/Documents/eecs583/workspace/project/featureExtractor
make -C /home/funke/Documents/eecs583/workspace/project/featureExtractor

rm train_x.csv
rm llvmprof.out # Otherwise your profile runs are added together
clang -emit-llvm -o $fname.bc -c $fname.c || { echo "Failed to emit llvm bc"; exit 1; }
opt -loop-simplify < $fname.bc > $fname.ls.bc || { echo "Failed to opt loop simplify"; exit 1; }
opt -insert-edge-profiling $fname.ls.bc -o $fname.profile.ls.bc
llc $fname.profile.ls.bc -o $fname.profile.ls.s
g++ -o $fname.profile $fname.profile.ls.s /home/funke/Documents/eecs583/llvm/build/Release+Asserts/lib/libprofile_rt.so

./$fname.profile $input> /dev/null

opt -load /home/funke/Documents/eecs583/workspace/project/featureExtractor/Release+Asserts/lib/libfeatureExtractor.so -lamp-insts -insert-lamp-profiling -insert-lamp-loop-profiling -insert-lamp-init < $fname.ls.bc > $fname.lamp.bc || { echo "Failed to get features"; exit 1; }
llc < $fname.lamp.bc > $fname.lamp.s || { echo "Failed to llc"; exit 1; }
g++ -o $fname.lamp.exe $fname.lamp.s /home/funke/Documents/eecs583/workspace/project/featureExtractor/tools/lamp-profiler/lamp_hooks.o || { echo "Failed to add lamp hooks"; exit 1; }

echo "executing lamp exe"
./$fname.lamp.exe $input> /dev/null
echo "loading pass"
opt -analyze -mem2reg -indvars -scalar-evolution -stats -load /home/funke/Documents/eecs583/workspace/project/featureExtractor/Release+Asserts/lib/libfeatureExtractor.so -lamp-inst-cnt -lamp-map-loop -lamp-load-profile -profile-loader -profile-info-file=llvmprof.out -featsExtractor -benchmark $fname -output-filename train_x.csv < $fname.ls.bc > $fname.ls.feats.bc || { echo "Failed to get feature stats"; exit 1; }