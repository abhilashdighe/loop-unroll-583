#!/bin/bash

cd ./CINT2006/456.hmmer
./run_feature.sh
cd ../../

cd ./CFP2006/447.dealII
./run_feature.sh
cd ../../

cd ./CINT2006/456.hmmer
./run_unroll.sh 1
cd ../../
cd ./CINT2006/456.hmmer
./run_unroll.sh 2
cd ../../
cd ./CINT2006/456.hmmer
./run_unroll.sh 3
cd ../../
cd ./CINT2006/456.hmmer
./run_unroll.sh 4
cd ../../
cd ./CINT2006/456.hmmer
./run_unroll.sh 5
cd ../../
cd ./CINT2006/456.hmmer
./run_unroll.sh 6
cd ../../

