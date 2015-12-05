/*
Fast Artificial Neural Network Library (fann)
Copyright (C) 2003-2012 Steffen Nissen (sn@leenissen.dk)

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include <stdio.h>
#include "fann.h"

int main()
{	const unsigned int num_layers = 3;
	const unsigned int num_neurons_hidden = 32;
	const float desired_error = (const float) 0.0001;
	const unsigned int max_epochs = 300;
	const unsigned int epochs_between_reports = 10;
	struct fann *ann;
	struct fann_train_data *train_data;

	unsigned int i = 0;

	printf("Creating network.\n");

	train_data = fann_read_train_from_file("data.train");

	ann = fann_create_standard(num_layers,
					  train_data->num_input, num_neurons_hidden, train_data->num_output);

	printf("Training network.\n");

	fann_set_activation_function_hidden(ann, FANN_SIGMOID_SYMMETRIC_STEPWISE);
	fann_set_activation_function_output(ann, FANN_SIGMOID_STEPWISE);

	/*fann_set_training_algorithm(ann, FANN_TRAIN_INCREMENTAL); */

	fann_train_on_data(ann, train_data, max_epochs, epochs_between_reports, desired_error);

	fann_save(ann, "trained.net");


	printf("Cleaning up.\n");
	fann_destroy_train(train_data);
	fann_destroy(ann);

	return 0;
}
