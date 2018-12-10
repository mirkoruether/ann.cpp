#include "cost_function.h"
#include "mat_arr_math.h"
#include <cmath>

using namespace linalg;
using namespace annlib;

void cost_function::calculate_output_layer_error(const mat_arr& net_output_rv,
                                                 const mat_arr& solution_rv,
                                                 const mat_arr& output_layer_weighted_input_rv,
                                                 const std::function<float(float)>& derivative_activation_function,
                                                 mat_arr* output_layer_error_rv) const
{
	calculate_gradient(net_output_rv, solution_rv, output_layer_error_rv);

	mat_element_by_element_operation(*output_layer_error_rv, output_layer_weighted_input_rv, output_layer_error_rv,
	                                 [derivative_activation_function](float grad, float wi)
	                                 {
		                                 return grad * derivative_activation_function(wi);
	                                 });
}

float quadratic_costs::calculate_costs(const mat_arr& net_output_rv, const mat_arr& solution_rv) const
{
	const size_t size = net_output_rv.size();
	if (solution_rv.size() != size)
	{
		throw std::runtime_error("sizes differ");
	}

	const float* no_element = net_output_rv.start();
	const float* s_element = solution_rv.start();

	float result = 0.0;
	for (unsigned i = 0; i < size; i++)
	{
		result += (*no_element - *s_element) * (*no_element - *s_element);
		no_element++;
		s_element++;
	}
	return result;
}

void quadratic_costs::calculate_gradient(const mat_arr& net_output_rv,
                                         const mat_arr& solution_rv,
                                         mat_arr* gradient_rv) const
{
	mat_element_wise_sub(net_output_rv, solution_rv, gradient_rv);
}

float cross_entropy_costs::calculate_costs(const mat_arr& net_output_rv,
                                            const mat_arr& solution_rv) const
{
	const size_t size = net_output_rv.size();
	if (solution_rv.size() != size)
	{
		throw std::runtime_error("sizes differ");
	}

	const float* no_element = net_output_rv.start();
	const float* s_element = solution_rv.start();

	float result = 0;
	for (unsigned i = 0; i < size; i++)
	{
		const float a = *no_element;
		const float y = *s_element;
		result += y * std::log(a) + (1 - y) * std::log(1 - a);

		no_element++;
		s_element++;
	}
	return -result;
}

struct calculate_gradient_kernel
{
	float operator()(float a, float y) const 
	{
		return y / a - (1.0f - y) / (1.0f - a);
	}
};

void cross_entropy_costs::calculate_gradient(const mat_arr& net_output_rv,
                                             const mat_arr& solution_rv,
                                             mat_arr* gradient_rv) const
{
	mat_element_by_element_operation(net_output_rv, solution_rv, gradient_rv,
									 calculate_gradient_kernel());
}

void cross_entropy_costs::calculate_output_layer_error(const mat_arr& net_output_rv,
                                                       const mat_arr& solution_rv,
                                                       const mat_arr& output_layer_weighted_input_rv,
                                                       const std::function<float(float)>& derivative_activation_function,
                                                       mat_arr* output_layer_error_rv) const
{
	//TODO Check for logistic activation function
	mat_element_wise_sub(net_output_rv, solution_rv, output_layer_error_rv);
}
