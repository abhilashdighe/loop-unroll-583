#!/bin/bash

cd ./CFP2006/470.lbm
./run_feature.sh
cd ../../

cd ./CFP2006/450.soplex
./run_feature.sh
cd ../../

cd ./CINT2006/403.gcc
./run_feature.sh
cd ../../

cd ./CFP2006/470.lbm
./run_unroll.sh 1
cd ../../

cd ./CFP2006/470.lbm
./run_unroll.sh 2
cd ../../
cd ./CFP2006/470.lbm
./run_unroll.sh 3
cd ../../
cd ./CFP2006/470.lbm
./run_unroll.sh 4
cd ../../
cd ./CFP2006/470.lbm
./run_unroll.sh 5
cd ../../
cd ./CFP2006/470.lbm
./run_unroll.sh 6
cd ../../

cd ./CFP2006/450.soplex
./run_unroll.sh 1
cd ../../

cd ./CFP2006/450.soplex
./run_unroll.sh 2
cd ../../

cd ./CFP2006/450.soplex
./run_unroll.sh 3
cd ../../

cd ./CFP2006/450.soplex
./run_unroll.sh 4
cd ../../

cd ./CFP2006/450.soplex
./run_unroll.sh 5
cd ../../

cd ./CFP2006/450.soplex
./run_unroll.sh 6
cd ../../



