fann: 
	gcc -O3 neural_nets_fann/train_nn.c -o neural_nets_fann/train_nn -lfann -lm
	gcc -O3 neural_nets_fann/test_nn.c -o neural_nets_fann/test_nn -lfann -lm
	cd neural_nets_fann && ./train_nn
	cd neural_nets_fann && ./test_nn

gather-results:
	python data_conversion/convert_results.py

clean-fann:
	rm -f neural_nets_fann/train_nn neural_nets_fann/test_nn neural_nets_fann/trained.net	

nn:
	python all_classifiers.py nn

rf:
	python all_classifiers.py rf

lr:
	python all_classifiers.py lr

svm:
	python all_classifiers.py svm