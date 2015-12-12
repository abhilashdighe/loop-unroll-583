#!/bin/bash

# NOTE:
# ******************************************
# 1) LLVM_HOME = PATH TO LLVM HOME DIR
#    Set and export these environment variables in your /etc/profile OR /etc/bash.rc files

make clean
make TEST=simplify

unroll_factors_file="factors.csv"
label_file="labels.csv"
# Replace with bench
project_name="loop_unroll"
PASS_HOME="/home/vgogte/eecs583/loop-unroll-583/project/loop_unroll"

fname="464.h264ref"
INPUTFILE="/work/lnt-llvm/SPEC/speccpu2006/benchspec/CPU2006/464.h264ref/data/test/input/foreman_test_encoder_baseline.cfg"
OUTPUTFILE=""
STDOUTFILE="h264ref.test.foreman_baseline.out"
STDERRFILE="h264ref.test.foreman_baseline.err"						
ARGUMENTS=" -d $(basename "$INPUTFILE")"

PATH=$PATH:/home/vgogte/build/Release+Asserts/bin

cd Output

cp /home/vgogte/perlbench_preds.csv ./$unroll_factors_file

echo "simplifying and rotating loop"
opt -loop-simplify -mem2reg -loop-rotate -indvars < $fname.linked.rbc > $fname.rotate.ls.link.bc 

echo "getting labels before unrolling"
rm $label_file
opt -load $PASS_HOME/Release+Asserts/lib/$project_name.so -loop-label -label-outfile $label_file -benchmark $fname < $fname.rotate.ls.link.bc

echo "unrolling loop using default factors"
opt -loop-unroll < $fname.rotate.ls.link.bc > $fname.default.unroll.bc


echo "unrolling loop with best unroll factors"
opt -load $PASS_HOME/Release+Asserts/lib/$project_name.so -best-unroll -factors-file $unroll_factors_file -label-infile $label_file < $fname.rotate.ls.link.bc > $fname.best.unroll.bc

echo "Executing unrolled and instrumented benchmark"


llc $fname.linked.rbc -o $fname.s
g++ $fname.s -o $fname.exe
{ time ./$fname.exe $ARGUMENTS; } 2> baseline.time

llc $fname.default.unroll.bc -o $fname.s
g++ $fname.s -o $fname.exe
{ time ./$fname.exe $ARGUMENTS; } 2> default.time



llc $fname.best.unroll.bc -o $fname.s
g++ $fname.s -o $fname.exe
{ time ./$fname.exe $ARGUMENTS; } 2> best_unroll.time

