make clean
make TEST=unroll
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
cd Output

opt -insert-edge-profiling $fname.linked.rbc -o $fname.profile.ls.bc
llc $fname.profile.ls.bc -o $fname.profile.ls.s
g++ -o $fname.profile $fname.profile.ls.s /home/vgogte/build/Release+Asserts/lib/libprofile_rt.so
./$fname.profile $ARGUMENTS 

echo "profiling array element reuses"
opt -load $PASS_HOME/Release+Asserts/lib/$project_name.so -arrayElemReuseProfile -benchmark  -reuse-filename temp_array_reuse.csv < $fname.linked.rbc

echo "extracting features"
opt -analyze -mem2reg -indvars -scalar-evolution -stats < $fname.linked.rbc > $fname.ls.temp.bc
opt -load $PASS_HOME/Release+Asserts/lib/$project_name.so -profile-loader -profile-info-file=llvmprof.out -featsExtractor -benchmark $fname -output-filename $train_file < $fname.ls.temp.bc > $fname.ls.feats.bc

#llc 400.perlbench.linked.rbc -o 400.perlbench.s
#g++ 400.perlbench.s -o 400.perlbench.exe
#./400.perlbench.exe $ARGUMENTS 
