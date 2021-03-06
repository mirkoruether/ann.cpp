#include "weight_norm_penalty.h"
#include "mat_arr_math.h"
#include "mat_arr_math_t.h"
#include <functional>

#ifdef ANNLIB_USE_CUDA
#include "weight_norm_penalty_cudaops.cuh"
#endif

using namespace annlib;

abstract_weight_norm_penalty::abstract_weight_norm_penalty(fpt regularization_parameter)
	: regularization_parameter(regularization_parameter)
{
}

void abstract_weight_norm_penalty::add_penalty_to_gradient(const mat_arr& weights_noarr,
                                                           mat_arr* gradient_noarr) const
{
	add_penalty_to_gradient(weights_noarr, regularization_parameter, gradient_noarr);
}

L1_regularization::L1_regularization(fpt regularization_parameter)
	: abstract_weight_norm_penalty(regularization_parameter)
{
}

L1_regularization::L1_regularization(fpt normalized_regularization_parameter,
                                     unsigned training_set_size)
	: abstract_weight_norm_penalty(normalized_regularization_parameter / training_set_size)
{
}

void L1_regularization::add_penalty_to_gradient(const mat_arr& weights_noarr,
                                                fpt regularization_parameter,
                                                mat_arr* gradient_noarr) const
{
#ifdef ANNLIB_USE_CUDA
	cuda::cuda_add_L1_regularization(weights_noarr, regularization_parameter, gradient_noarr);
#else
	const std::function<fpt(fpt)> sgn = [](fpt x) { return x > 0 ? 1.0f : -1.0f; };
	mat_element_by_element_operation(*gradient_noarr, weights_noarr, gradient_noarr,
	                                 [&](fpt d, fpt w)
	                                 {
		                                 return d + regularization_parameter * sgn(w);
	                                 });
#endif
}

L2_regularization::L2_regularization(fpt regularization_parameter)
	: abstract_weight_norm_penalty(regularization_parameter)
{
}

L2_regularization::L2_regularization(fpt normalized_regularization_parameter,
                                     unsigned training_set_size)
	: abstract_weight_norm_penalty(normalized_regularization_parameter / training_set_size)
{
}

void L2_regularization::add_penalty_to_gradient(const mat_arr& weights_noarr,
                                                fpt regularization_parameter,
                                                mat_arr* gradient_noarr) const
{
#ifdef ANNLIB_USE_CUDA
	cuda::cuda_add_L2_regularization(weights_noarr, regularization_parameter, gradient_noarr);
#else
	mat_element_by_element_operation(*gradient_noarr, weights_noarr, gradient_noarr,
	                                 [&](fpt d, fpt w)
	                                 {
		                                 return d + regularization_parameter * w;
	                                 });
#endif
}
