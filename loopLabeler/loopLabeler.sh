project_dir=$1
fname=$2

make clean -C $project_dir
make -C $project_dir

rm llvmprof.out # Otherwise your profile runs are added together
clang -emit-llvm -o $fname.bc -c $fname.c || { echo "Failed to emit llvm bc"; exit 1; }

#loop simplify just to make sure that no new loops get introduced (ideally they won't)
opt -loop-simplify < $fname.bc > $fname.ls.bc || { echo "Failed to opt loop simplify"; exit 1; }

#label loops
opt -load $project_dir/Release+Asserts/lib/libfeatureExtractor.so -loop-label -benchmark $fname < $fname.ls.bc > $fname.ls.label.bc || { echo "Failed to get ids for loops"; exit 1; }
