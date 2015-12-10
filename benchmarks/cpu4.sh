#!/bin/bash

cd ./CINT2006/458.sjeng
./run_feature.sh
cd ../../

cd ./CINT2006/471.omnetpp
./run_feature.sh
cd ../../


cd ./CINT2006/458.sjeng
./run_unroll.sh 1
cd ../../
cd ./CINT2006/458.sjeng
./run_unroll.sh 2
cd ../../

cd ./CINT2006/458.sjeng
./run_unroll.sh 3
cd ../../
cd ./CINT2006/458.sjeng
./run_unroll.sh 4
cd ../../


cd ./CINT2006/458.sjeng
./run_unroll.sh 5
cd ../../
cd ./CINT2006/458.sjeng
./run_unroll.sh 6
cd ../../


cd ./CINT2006/471.omnetpp
./run_unroll.sh 1
cd ../../
cd ./CINT2006/471.omnetpp
./run_unroll.sh 2
cd ../../
cd ./CINT2006/471.omnetpp
./run_unroll.sh 3
cd ../../
cd ./CINT2006/471.omnetpp
./run_unroll.sh 4
cd ../../
cd ./CINT2006/471.omnetpp
./run_unroll.sh 5
cd ../../
cd ./CINT2006/471.omnetpp
./run_unroll.sh 6
cd ../../


