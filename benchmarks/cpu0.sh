#!/bin/bash

cd ./CINT2006/429.mcf
./run_feature.sh
cd ../../

cd ./CFP2006/444.namd
./run_feature.sh
cd ../../

cd ./CINT2006/483.xalancbmk
./run_feature.sh
cd ../../

cd ./CINT2006/429.mcf
./run_unroll.sh 1
cd ../../
cd ./CINT2006/429.mcf
./run_unroll.sh 2
cd ../../

cd ./CINT2006/429.mcf
./run_unroll.sh 3
cd ../../
cd ./CINT2006/429.mcf
./run_unroll.sh 4
cd ../../
cd ./CINT2006/429.mcf
./run_unroll.sh 5
cd ../../
cd ./CINT2006/429.mcf
./run_unroll.sh 6
cd ../../

cd ./CFP2006/444.namd
./run_unroll.sh 1
cd ../../

cd ./CFP2006/444.namd
./run_unroll.sh 2
cd ../../

cd ./CFP2006/444.namd
./run_unroll.sh 3
cd ../../
cd ./CFP2006/444.namd
./run_unroll.sh 4
cd ../../
cd ./CFP2006/444.namd
./run_unroll.sh 5
cd ../../
cd ./CFP2006/444.namd
./run_unroll.sh 6
cd ../../

