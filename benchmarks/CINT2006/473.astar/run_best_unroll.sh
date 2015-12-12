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

fname="473.astar"
INPUTFILE="/home/vgogte/spec2006/benchspec/CPU2006/473.astar/data/test/input/lake.cfg"
OUTPUTFILE=""
STDOUTFILE="astar.test.lake.out"
STDERRFILE="astar.test.lake.err"						
ARGUMENTS=" $(basename "$INPUTFILE")"

NTS=" -I/home/vgogte/spec2006/benchspec/CPU2006/400.perlbench/data/all/input/lib $INPUTFILE 4 800 10 17 19 300"

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

