#ifndef COST_FUNCTION_H
#define COST_FUNCTION_H

#include "mat_arr.h"
#include "activation_function.h"

using namespace linalg;

namespace annlib
{
	class cost_function
	{
	public:
		virtual ~cost_function() = default;

		virtual float calculate_costs(const mat_arr& net_output_rv,
		                               const mat_arr& solution_rv) const = 0;

		virtual void calculate_gradient(const mat_arr& net_output_rv,
		                                const mat_arr& solution_rv,
		                                mat_arr* gradient_rv) const = 0;

		virtual void calculate_output_layer_error(const mat_arr& net_output_rv, const mat_arr& solution_rv,
		                                          const mat_arr& output_layer_weighted_input_rv,
		                                          const activation_function& activation_function,
		                                          mat_arr* output_layer_error_rv) const;
	};

	class quadratic_costs : public cost_function
	{
		float calculate_costs(const mat_arr& net_output_rv,
		                       const mat_arr& solution_rv) const override;

		void calculate_gradient(const mat_arr& net_output_rv,
		                        const mat_arr& solution_rv,
		                        mat_arr* gradient_rv) const override;
	};

	class cross_entropy_costs : public cost_function
	{
		float calculate_costs(const mat_arr& net_output_rv,
		                       const mat_arr& solution_rv) const override;

		void calculate_gradient(const mat_arr& net_output_rv,
		                        const mat_arr& solution_rv,
		                        mat_arr* gradient_rv) const override;

		void calculate_output_layer_error(const mat_arr& net_output_rv,
		                                  const mat_arr& solution_rv,
		                                  const mat_arr& output_layer_weighted_input_rv,
		                                  const activation_function& activation_function,
		                                  mat_arr* output_layer_error_rv) const override;
	};
}
#endif
