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
{

    struct fann *ann;
    struct fann_train_data *test_data;

    unsigned int i = 0;

    test_data = fann_read_train_from_file("data.test");
    ann = fann_create_from_file("trained.net");

    printf("Testing network.\n");


    fann_reset_MSE(ann);
    for(i = 0; i < fann_length_train_data(test_data); i++)
    {
        fann_test(ann, test_data->input[i], test_data->output[i]);
    }
    
    printf("MSE error on test data: %f\n", fann_get_MSE(ann));

    printf("Cleaning up.\n");
    fann_destroy_train(test_data);
    fann_destroy(ann);
	return 0;
}
