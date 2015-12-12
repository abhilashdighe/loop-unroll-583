#!/bin/bash
declare -a benchmarks=( '464.h264ref' '403.gcc' '444.namd' '483.xalancbmk' '450.soplex' '473.astar' '471.omnetpp' '433.milc'  '400.perlbench' '447.dealII' )
declare -a methods=( 'rf' 'lr' 'svm' 'random' )

for b in "${benchmarks[@]}"
do
    echo $b
    python data_conversion/convert_results.py $b
    for m in "${methods[@]}"
    do
        echo $m
        python all_classifiers.py $m
    done
    echo ''
done

