make clean
make TEST=simplify
train_file="train.txt"
project_name="loop_unroll"
fname="400.perlbench"
PASS_HOME="/home/vgogte/eecs583/loop-unroll-583/project/loop_unroll"
INPUTFILE="/home/vgogte/spec2006/benchspec/CPU2006/400.perlbench/data/all/input/diffmail.pl"
OUTPUTFILE=""
STDOUTFILE="perlbench.all.diffmail.out"
STDERRFILE="dperlbench.ref.diffmail.err"						
ARGUMENTS=" -I/home/vgogte/spec2006/benchspec/CPU2006/400.perlbench/data/all/input/lib $INPUTFILE 4 800 10 17 19 300 > $STDOUTFILE 2> $STDERRFILE "
PATH=$PATH:/home/vgogte/build/Release+Asserts/bin

unroll_factor=2

cd Output

rm loopTimings.csv
echo "simplifying and rotating loop"
opt  -loop-simplify -loop-rotate < $fname.linked.rbc > $fname.rotate.ls.link.bc

echo "instrumenting loop"
opt -load $PASS_HOME/Release+Asserts/lib/$project_name.so -benchmark $fname -module-inst -loop-inst < $fname.rotate.ls.link.bc > $fname.li.unroll.rotate.ls.link.bc



echo "unrolling loop"
opt -mem2reg -load $PASS_HOME/Release+Asserts/lib/$project_name.so -benchmark $fname -debug -custom-unroll -custom-count $unroll_factor < $fname.li.unroll.rotate.ls.link.bc > $fname.unroll.rotate.ls.link.bc

clang -std=c++11 -emit-llvm -o timer.bc -c $PASS_HOME/lib/$project_name/timerFuncs.cpp || { echo "Failed to emit llvm bc for timers"; exit 1; }
llvm-link timer.bc $fname.unroll.rotate.ls.link.bc -S -o=$fname.op

llc $fname.op -o $fname.op.s
g++ $fname.op.s -o 400.perlbench.op.exe
./400.perlbench.op.exe $ARGUMENTS|tee 400.perlbench.log 
