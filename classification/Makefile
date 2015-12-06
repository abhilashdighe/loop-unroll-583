fann: 
	gcc -O3 neural_nets_fann/train_nn.c -o neural_nets_fann/train_nn -lfann -lm
	gcc -O3 neural_nets_fann/test_nn.c -o neural_nets_fann/test_nn -lfann -lm
	cd neural_nets_fann && ./train_nn
	cd neural_nets_fann && ./test_nn

clean-fann:
	rm -f neural_nets_fann/train_nn neural_nets_fann/test_nn neural_nets_fann/trained.net	

nn:
	python neural_nets/skflowNN.py data.train data.test

rf:
	python random_forests/classifyRF.py data.train data.test
	

lr:
	python logistic_regression/classifyLR.py data.train data.test


svm:
	python support_vector/classifySVM.py data.train data.test