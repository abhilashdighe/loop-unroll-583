#!/bin/bash

cd ./CINT2006/401.bzip2
./run_feature.sh
cd ../../

cd ./CINT2006/445.gobmk
./run_feature.sh
cd ../../

cd ./CINT2006/401.bzip2
./run_unroll.sh 1
cd ../../

cd ./CINT2006/401.bzip2
./run_unroll.sh 2
cd ../../
cd ./CINT2006/401.bzip2
./run_unroll.sh 3
cd ../../
cd ./CINT2006/401.bzip2
./run_unroll.sh 4
cd ../../
cd ./CINT2006/401.bzip2
./run_unroll.sh 5
cd ../../
cd ./CINT2006/401.bzip2
./run_unroll.sh 6
cd ../../

cd ./CINT2006/445.gobmk
./run_unroll.sh 1
cd ../../

cd ./CINT2006/445.gobmk
./run_unroll.sh 2
cd ../../
cd ./CINT2006/445.gobmk
./run_unroll.sh 3
cd ../../
cd ./CINT2006/445.gobmk
./run_unroll.sh 4
cd ../../
cd ./CINT2006/445.gobmk
./run_unroll.sh 5
cd ../../
cd ./CINT2006/445.gobmk
./run_unroll.sh 6
cd ../../

