#!/bin/bash
make clean
make TEST=simplify
train_file="train$1\.txt"
project_name="loop_unroll"
PASS_HOME="/home/vgogte/eecs583/loop-unroll-583/project/loop_unroll"

fname="470.lbm"
INPUTFILE="/home/vgogte/spec2006/benchspec/CPU2006/470.lbm/data/test/input/100_100_130_cf_a.of"
OUTPUTFILE=""
STDOUTFILE="lbm.ref.out"
STDERRFILE="lbm.ref.err"						
ARGUMENTS="1000 reference.dat 0 0 $INPUTFILE"

PATH=$PATH:/home/vgogte/build/Release+Asserts/bin
unroll_factor=$1

cd Output

rm loopTimings.csv
echo "simplifying and rotating loop"
opt  -loop-simplify -loop-rotate < $fname.linked.rbc > $fname.rotate.ls.link.bc

echo "instrumenting loop"
opt -load $PASS_HOME/Release+Asserts/lib/$project_name.so -benchmark $fname -module-inst -loop-inst < $fname.rotate.ls.link.bc > $fname.li.unroll.rotate.ls.link.bc



echo "unrolling loop"
opt -mem2reg -load $PASS_HOME/Release+Asserts/lib/$project_name.so -benchmark $fname -custom-unroll -custom-count $unroll_factor < $fname.li.unroll.rotate.ls.link.bc > $fname.unroll.rotate.ls.link.bc


llc $fname.unroll.rotate.ls.link.bc -o $fname.op.s
g++ -o $fname.op.exe $fname.op.s /home/vgogte/timerFuncs.so
./$fname.op.exe $ARGUMENTS|tee $fname.log 
cp loopTimings.csv $train_file

cp $train_file ~/eecs583/loop-unroll-583/results/$fname\_$train_file\.csv

