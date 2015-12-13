make clean
make TEST=simplify
train_file="train.txt"
project_name="loop_unroll"
PASS_HOME="/home/vgogte/eecs583/loop-unroll-583/project/loop_unroll"

fname="429.mcf"
INPUTFILE="/home/vgogte/spec2006/benchspec/CPU2006/429.mcf/data/test/input/inp.in"
OUTPUTFILE=""
STDOUTFILE="mcf.test.out"
STDERRFILE="mcf.test.err"						
ARGUMENTS=" $INPUTFILE > $STDOUTFILE 2> $STDERRFILE "

PATH=$PATH:/home/vgogte/build/Release+Asserts/bin
cd Output
rm temp_array_reuse.csv
opt -insert-edge-profiling $fname.linked.rbc -o $fname.profile.ls.bc
llc $fname.profile.ls.bc -o $fname.profile.ls.s
g++ -o $fname.profile $fname.profile.ls.s /home/vgogte/build/Release+Asserts/lib/libprofile_rt.so
./$fname.profile $ARGUMENTS 

echo "profiling array element reuses"
opt -load $PASS_HOME/Release+Asserts/lib/$project_name.so -arrayElemReuseProfile -benchmark $fname -reuse-filename temp_array_reuse.csv < $fname.linked.rbc

echo "extracting features"
opt -analyze -mem2reg -indvars -scalar-evolution -stats -load $PASS_HOME/Release+Asserts/lib/$project_name.so -profile-loader -profile-info-file=llvmprof.out -featsExtractor -benchmark $fname -output-filename $train_file < $fname.linked.rbc > $fname.ls.feats.bc

#llc $fname.linked.rbc -o $fname.s
#g++ $fname.s -o $fname.exe
#./$fname.exe $ARGUMENTS 
