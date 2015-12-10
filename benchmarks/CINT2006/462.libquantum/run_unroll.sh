#!/bin/bash
make clean
make TEST=simplify
train_file="train$1\.csv"
project_name="loop_unroll"
PASS_HOME="/home/vgogte/eecs583/loop-unroll-583/project/loop_unroll"

fname="462.libquantum"
OUTPUTFILE=""
STDOUTFILE="libquantum.test.out"
STDERRFILE="libquantum.test.err"					
ARGUMENTS=" 33 5"

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

