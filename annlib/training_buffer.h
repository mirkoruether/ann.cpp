#ifndef TRAINING_BUFFER_H
#define TRAINING_BUFFER_H

#include "mat_arr.h"

using namespace linalg;

namespace annlib
{
	class partial_training_buffer;

	class training_buffer
	{
	public:
		training_buffer(vector<unsigned> sizes, unsigned mini_batch_size, unsigned part_count);

		mat_arr input_rv;
		mat_arr solution_rv;

		vector<mat_arr> weighted_inputs_rv;
		vector<mat_arr> activations_rv;
		vector<mat_arr> errors_rv;

		vector<mat_arr> gradient_biases_rv_noarr;
		vector<mat_arr> gradient_weights_noarr;

		vector<partial_training_buffer> partial_buffers;

		vector<mat_arr*> all();
		void clear();

		unsigned layer_count() const;
		unsigned part_count() const;
	};

	class partial_training_buffer
	{
	public:
		partial_training_buffer(training_buffer* buf,
		                        unsigned start, unsigned count);

		mat_arr input_rv;
		mat_arr solution_rv;

		vector<mat_arr> weighted_inputs_rv;
		vector<mat_arr> activations_rv;
		vector<mat_arr> errors_rv;
	};
}
#endif