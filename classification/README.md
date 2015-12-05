# Classification
Contains code for learning a multiclass classifier for predicting the unrolling factor.

## Format
NumSamples NumFeatures NumOutputs(8)

f1 f2 f3 f4 f5 f6 f7 f8 f9 f10 f11 f12 f13 f14 f15 .... f22 unroll_factor  
f1 f2 f3 f4 f5 f6 f7 f8 f9 f10 f11 f12 f13 f14 f15 .... f22 unroll_factor  
f1 f2 f3 f4 f5 f6 f7 f8 f9 f10 f11 f12 f13 f14 f15 .... f22 unroll_factor  
f1 f2 f3 f4 f5 f6 f7 f8 f9 f10 f11 f12 f13 f14 f15 .... f22 unroll_factor  
f1 f2 f3 f4 f5 f6 f7 f8 f9 f10 f11 f12 f13 f14 f15 .... f22 unroll_factor  
f1 f2 f3 f4 f5 f6 f7 f8 f9 f10 f11 f12 f13 f14 f15 .... f22 unroll_factor  
...

Unroll Factor refers to the unrolling factor which performs best for these features.

## FANN: 

**Dependencies**  
[Download here](http://sourceforge.net/projects/fann/files/fann/2.2.0/FANN-2.2.0-Source.zip/download)
Inside the folder run:
```
cmake .
sudo make install
sudo ldconfig
```

**Usage**  
Create train and test files in the format as given in files neural_nets_fann/data.train and neural_nets_fann/data.train.  
Run the following commands.
```
make fann
make clean-fann
```

A file called trained.net will be created. Use that file in a way similar to the way it's used in test_nn.c in optimization passes. 


## Python Classifiers:

**Dependencies**  
scikit-learn
numpy  

**Usage**
Create data.train and data.test according to the guidelines and run the following commands.  
'''
make svm
make lr
make rf
'''