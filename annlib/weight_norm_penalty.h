#ifndef WEIGHT_NORM_PENALTY_H
#define WEIGHT_NORM_PENALTY_H

#include "mat_arr.h";

using namespace linalg;

namespace annlib
{
	class weight_norm_penalty
	{
	public:
		virtual ~weight_norm_penalty() = default;
		virtual void add_penalty_to_gradient(const mat_arr& weights_noarr,
		                                     mat_arr* gradient_noarr) const = 0;
	};

	class abstract_weight_norm_penalty : public weight_norm_penalty
	{
	protected:
		explicit abstract_weight_norm_penalty(double regularization_parameter);

	public:
		double regularization_parameter;

		virtual void add_penalty_to_gradient(const mat_arr& weights_noarr,
		                                     mat_arr* decay_noarr) const override;

		virtual void add_penalty_to_gradient(const mat_arr& weights_noarr,
		                                     double regularization_parameter,
		                                     mat_arr* gradient_noarr) const = 0;
	};

	class L1_regularization : public abstract_weight_norm_penalty
	{
	public:
		explicit L1_regularization(double regularization_parameter);

		L1_regularization(double normalized_regularization_parameter, unsigned training_set_size);

		void add_penalty_to_gradient(const mat_arr& weights_noarr,
		                             double regularization_parameter,
		                             mat_arr* gradient_noarr) const override;
	};

	class L2_regularization : public abstract_weight_norm_penalty
	{
	public:
		explicit L2_regularization(double regularization_parameter);

		L2_regularization(double normalized_regularization_parameter, unsigned training_set_size);

		void add_penalty_to_gradient(const mat_arr& weights_noarr,
		                             double regularization_parameter,
		                             mat_arr* gradient_noarr) const override;
	};
}
#endif