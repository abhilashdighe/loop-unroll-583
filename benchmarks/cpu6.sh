#!/bin/bash

cd ./CFP2006/433.milc
./run_feature.sh
cd ../../

cd ./CINT2006/400.perlbench
./run_feature.sh
cd ../../

cd ./CFP2006/433.milc
./run_unroll.sh 1
cd ../../
cd ./CFP2006/433.milc
./run_unroll.sh 2
cd ../../
cd ./CFP2006/433.milc
./run_unroll.sh 3
cd ../../
cd ./CFP2006/433.milc
./run_unroll.sh 4
cd ../../
cd ./CFP2006/433.milc
./run_unroll.sh 5
cd ../../
cd ./CFP2006/433.milc
./run_unroll.sh 6
cd ../../



cd ./CINT2006/400.perlbench
./run_unroll.sh 1
cd ../../
cd ./CINT2006/400.perlbench
./run_unroll.sh 2
cd ../../
cd ./CINT2006/400.perlbench
./run_unroll.sh 3
cd ../../
cd ./CINT2006/400.perlbench
./run_unroll.sh 4
cd ../../
cd ./CINT2006/400.perlbench
./run_unroll.sh 5
cd ../../
cd ./CINT2006/400.perlbench
./run_unroll.sh 6
cd ../../
