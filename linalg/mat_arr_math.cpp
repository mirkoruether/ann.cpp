#define MAT_ARR_MATH_SIZE_CHECK 1

#include "mat_arr_math.h"
#include "general_util.h"

using namespace linalg;

namespace linalg
{
#pragma region multiple_element_by_element_internal


	void __mat_m_e_by_e_size_check(const vector<mat_arr*>& input, mat_arr* C)
	{
		const auto input_count = static_cast<unsigned>(input.size());
		for (unsigned i = 0; i < input_count; i++)
		{
			if (C->rows != input[i]->rows)
			{
				throw runtime_error("Row count does not fit");
			}

			if (C->cols != input[i]->cols)
			{
				throw runtime_error("Column count does not fit");
			}

			if (input[i]->count != 1 && C->cols != input[i]->cols)
			{
				throw runtime_error("Wrong input array sizes");
			}
		}
	}

	void __mat_multiple_e_by_e_operation(const vector<mat_arr*>& input, mat_arr* C,
	                                     const function<double(vector<double>)>& f)
	{
#ifdef MAT_ARR_MATH_SIZE_CHECK
		__mat_m_e_by_e_size_check(input, C);
#endif

		const unsigned rows_cols = C->rows * C->cols;
		const unsigned count = C->count;
		const auto input_count = static_cast<unsigned>(input.size());

		auto input_starts = vector_select<mat_arr*, double*>(input, [](mat_arr* x) { return x->start(); });

		double* c = C->start();

		for (unsigned mat_no = 0; mat_no < count; mat_no++)
		{
			for (unsigned rc = 0; rc < rows_cols; rc++)
			{
				*(c + rc) = f(vector_select<double*, double>(input_starts,
				                                             [&](const double* start)
				                                             {
					                                             return *(start + rc);
				                                             }));
			}

			for (unsigned i = 0; i < input_count; i++)
			{
				if (input[i]->count > 1)
				{
					input_starts[i] += rows_cols;
				}
			}
			c += rows_cols;
		}
	}
#pragma endregion
	mat_arr mat_multiple_e_by_e_operation(const vector<mat_arr*>& input, mat_arr* C,
	                                      const function<double(vector<double>)>& f)
	{
		if (C == nullptr)
		{
			const unsigned rows = input[0]->rows;
			const unsigned cols = input[0]->cols;
			unsigned count = input[0]->count;
			const auto input_count = static_cast<unsigned>(input.size());
			for (unsigned i = 0; i < input_count; i++)
			{
				if (input[i]->count > count)
				{
					count = input[i]->count;
				}
			}
			mat_arr tempC = mat_arr(count, rows, cols);
			__mat_multiple_e_by_e_operation(input, &tempC, f);
			return tempC;
		}
		__mat_multiple_e_by_e_operation(input, C, f);
		return *C;
	}

#pragma region element_by_element_internal
	inline void __e_by_e_size_check(const unsigned count_a, const unsigned rows_a, const unsigned cols_a,
	                                const unsigned count_b, const unsigned rows_b, const unsigned cols_b,
	                                const unsigned count_c, const unsigned rows_c, const unsigned cols_c)
	{
		if (count_a != 1 && count_b != 1 && count_a != count_b)
		{
			throw runtime_error("Wrong input array sizes");
		}

		if (count_c != max(count_a, count_b))
		{
			throw runtime_error("Wrong output array sizes");
		}

		if (rows_a != rows_b || rows_b != rows_c)
		{
			throw runtime_error("Row count does not fit");
		}

		if (cols_a != cols_b || cols_b != cols_c)
		{
			throw runtime_error("Column count does not fit");
		}
	}

	void __mat_element_by_element_operation(const mat_arr& A, const mat_arr& B, mat_arr* C,
	                                        const function<double(double, double)>& f,
	                                        const bool transpose_a, const bool transpose_b)
	{
		const unsigned count = C->count;
		const unsigned rows = C->rows;
		const unsigned cols = C->cols;

#ifdef MAT_ARR_MATH_SIZE_CHECK
		__e_by_e_size_check(A.count, transpose_a ? A.cols : A.rows, transpose_a ? A.rows : A.cols,
		                    B.count, transpose_b ? B.cols : B.rows, transpose_b ? B.rows : B.cols,
		                    count, rows, cols);
#endif

		double* c = C->start();

		const bool a_is_array = A.count > 1;
		const bool b_is_array = B.count > 1;
		const unsigned row_col = rows * cols;

		const double* a = A.start();
		const double* b = B.start();

		for (unsigned mat_no = 0; mat_no > count; ++mat_no)
		{
			unsigned i_normal = 0;
			unsigned i_transposed = 0;

			for (unsigned row = 0; row < rows; ++row)
			{
				for (unsigned col = 0; col < cols; ++col)
				{
					*(c + i_normal) = f(*(a + (transpose_a ? i_transposed : i_normal)),
					                    *(b + (transpose_b ? i_transposed : i_normal)));
					i_normal++;
					i_transposed += rows;
				}
				i_transposed -= (cols * rows - 1);
			}

			if (a_is_array)
				a += row_col;

			if (b_is_array)
				b += row_col;
		}
	}

	void __mat_element_by_element_operation(const mat_arr& A, const mat_arr& B, mat_arr* C,
	                                        const function<double(double, double)>& f, const mat_tr tr)
	{
		switch (tr)
		{
		case transpose_A:
			__mat_element_by_element_operation(A, B, C, f, true, false);
			return;
		case transpose_B:
			__mat_element_by_element_operation(A, B, C, f, false, true);
			return;
		case transpose_both:
			__mat_element_by_element_operation(A, B, C, f, true, true);
			return;
		default:
#ifdef MAT_ARR_MATH_SIZE_CHECK
			__e_by_e_size_check(A.count, A.rows, A.cols,
			                    B.count, B.rows, B.cols,
			                    C->count, C->rows, C->cols);
#endif
			const double* a = A.start();
			const double* b = B.start();
			double* c = C->start();

			const unsigned size_A = A.size();
			const unsigned size_B = B.size();
			const unsigned size_C = C->size();

			for (unsigned i = 0; i < size_C; ++i)
			{
				*(c + i) = f(*(a + (i % size_A)),
				             *(b + (i % size_B)));
			}
		}
	}
#pragma endregion

	mat_arr mat_element_by_element_operation(const mat_arr& A, const mat_arr& B, mat_arr* C,
	                                         const function<double(double, double)>& f, mat_tr tr)
	{
		if (C == nullptr)
		{
			const bool tr_A = (tr == transpose_A || tr == transpose_both);
			mat_arr tempC = mat_arr(max(A.count, B.count),
			                        tr_A ? A.cols : A.rows,
			                        tr_A ? A.rows : A.cols);
			__mat_element_by_element_operation(A, B, &tempC, f, tr);
			return tempC;
		}
		__mat_element_by_element_operation(A, B, C, f, tr);
		return *C;
	}

	mat_arr mat_element_wise_add(const mat_arr& A, const mat_arr& B, mat_arr* C, const mat_tr tr)
	{
		return mat_element_by_element_operation(A, B, C, [](double a, double b) { return a + b; }, tr);
	}

	mat_arr mat_element_wise_sub(const mat_arr& A, const mat_arr& B, mat_arr* C, const mat_tr tr)
	{
		return mat_element_by_element_operation(A, B, C, [](double a, double b) { return a - b; }, tr);
	}

	mat_arr mat_element_wise_mul(const mat_arr& A, const mat_arr& B, mat_arr* C, const mat_tr tr)
	{
		return mat_element_by_element_operation(A, B, C, [](double a, double b) { return a * b; }, tr);
	}

	mat_arr mat_element_wise_div(const mat_arr& A, const mat_arr& B, mat_arr* C, const mat_tr tr)
	{
		return mat_element_by_element_operation(A, B, C, [](double a, double b) { return a / b; }, tr);
	}

#pragma region element_wise_internal
	void __mat_element_wise_operation(const mat_arr& A, mat_arr* C,
	                                  const function<double(double)>& f)
	{
#ifdef MAT_ARR_MATH_SIZE_CHECK
		if (A.rows != C->rows
			|| A.cols != C->cols
			|| A.rows != C->rows)
		{
			throw runtime_error("Sizes do not fit");
		}
#endif

		const double* a = A.start();
		double* c = C->start();

		const unsigned size = C->size();
		for (unsigned i = 0; i < size; i++)
		{
			*(c + i) = f(*(a + i));
		}
	}
#pragma endregion

	mat_arr mat_element_wise_operation(const mat_arr& A, mat_arr* C, const function<double(double)>& f)
	{
		if (C == nullptr)
		{
			mat_arr tempC = mat_arr(A.count, A.rows, A.cols);
			__mat_element_wise_operation(A, &tempC, f);
			return tempC;
		}
		__mat_element_wise_operation(A, C, f);
		return *C;
	}

	mat_arr mat_element_wise_add(const mat_arr& A, double b, mat_arr* C)
	{
		return mat_element_wise_operation(A, C, [b](double a) { return a + b; });
	}

	mat_arr mat_element_wise_add(double a, const mat_arr& B, mat_arr* C)
	{
		return mat_element_wise_operation(B, C, [a](double b) { return a + b; });
	}

	mat_arr mat_element_wise_sub(const mat_arr& A, double b, mat_arr* C)
	{
		return mat_element_wise_operation(A, C, [b](double a) { return a - b; });
	}

	mat_arr mat_element_wise_sub(double a, const mat_arr& B, mat_arr* C)
	{
		return mat_element_wise_operation(B, C, [a](double b) { return a - b; });
	}

	mat_arr mat_element_wise_mul(const mat_arr& A, double b, mat_arr* C)
	{
		return mat_element_wise_operation(A, C, [b](double a) { return a * b; });
	}

	mat_arr mat_element_wise_mul(double a, const mat_arr& B, mat_arr* C)
	{
		return mat_element_wise_operation(B, C, [a](double b) { return a * b; });
	}

	mat_arr mat_element_wise_div(const mat_arr& A, double b, mat_arr* C)
	{
		return mat_element_wise_operation(A, C, [b](double a) { return a / b; });
	}

	mat_arr mat_element_wise_div(double a, const mat_arr& B, mat_arr* C)
	{
		return mat_element_wise_operation(B, C, [a](double b) { return a / b; });
	}

#pragma region matrix_mul_internal
	inline void __matrix_mul_size_check(const unsigned count_a, const unsigned rows_a, const unsigned cols_a,
	                                    const unsigned count_b, const unsigned rows_b, const unsigned cols_b,
	                                    const unsigned count_c, const unsigned rows_c, const unsigned cols_c)
	{
		if (count_a != 1 && count_b != 1 && count_a != count_b)
		{
			throw runtime_error("Wrong input array sizes");
		}

		if (count_c != max(count_a, count_b))
		{
			throw runtime_error("Wrong output array sizes");
		}

		if (cols_a != rows_b)
		{
			throw runtime_error("A and B cannot be multiplied");
		}

		if (rows_a != rows_c || cols_b != cols_c)
		{
			throw runtime_error("C has wrong size");
		}
	}

	void __mat_matrix_mul_add_case0(const mat_arr& A, const mat_arr& B, mat_arr* C)
	{
		const unsigned count = C->count;
		const unsigned l = A.rows;
		const unsigned m = A.cols;
		const unsigned n = B.cols;

		// Cache miss analysis: Inner Loop
		// A fixed, B row-wise, C row-wise
		for (unsigned matNo = 0; matNo < count; matNo++)
		{
			const double* a = A.start() + (matNo * l * m) % A.size();
			const double* b = B.start() + (matNo * m * n) % B.size();
			double* c = C->start() + (matNo * l * n);

			for (unsigned i = 0; i < l; i++)
			{
				const double* a_row = a + i * m;
				double* c_row = c + i * n;
				for (unsigned j = 0; j < m; j++)
				{
					const double a_val = *(a_row + j);
					const double* b_row = b + j * n;
					for (unsigned k = 0; k < n; k++)
					{
						*(c_row + k) += a_val * *(b_row + k);
					}
				}
			}
		}
	}

	void __mat_matrix_mul_add_case1(const mat_arr& A, const mat_arr& B, mat_arr* C)
	{
		const unsigned count = C->count;
		const unsigned l = A.cols;
		const unsigned m = A.rows;
		const unsigned n = B.cols;

		// Cache miss analysis: Inner Loop
		// A fixed, B row-wise, C row-wise
		for (unsigned matNo = 0; matNo < count; matNo++)
		{
			const double* a = A.start() + (matNo * l * m) % A.size();
			const double* b = B.start() + (matNo * m * n) % B.size();
			double* c = C->start() + (matNo * l * n);

			for (unsigned j = 0; j < m; j++)
			{
				const double* a_row = a + j * l;
				const double* b_row = b + j * n;
				for (unsigned i = 0; i < l; i++)
				{
					const double a_val = *(a_row + i);
					double* c_row = c + i * n;
					for (unsigned k = 0; k < n; k++)
					{
						*(c_row + k) += a_val * *(b_row + k);
					}
				}
			}
		}
	}

	void __mat_matrix_mul_add_case2(const mat_arr& A, const mat_arr& B, mat_arr* C)
	{
		const unsigned count = C->count;
		const unsigned l = A.rows;
		const unsigned m = A.cols;
		const unsigned n = B.rows;

		// Cache miss analysis: Inner Loop
		// A row-wise, B row-wise, C fixed
		for (unsigned matNo = 0; matNo < count; matNo++)
		{
			const double* a = A.start() + (matNo * l * m) % A.size();
			const double* b = B.start() + (matNo * m * n) % B.size();
			double* c = C->start() + (matNo * l * n);

			for (unsigned i = 0; i < l; i++)
			{
				const double* a_row = a + i * m;
				double* c_row = c + i * n;
				for (unsigned k = 0; k < n; k++)
				{
					const double* b_row = b + k * m;
					double c_val = *(c_row + k);
					for (unsigned j = 0; j < m; j++)
					{
						c_val += *(a_row + j) * *(b_row + j);
					}
					*(c_row + k) = c_val;
				}
			}
		}
	}

	void __mat_matrix_mul_add_case3(const mat_arr& A, const mat_arr& B, mat_arr* C)
	{
		const unsigned count = C->count;
		const unsigned l = A.cols;
		const unsigned m = A.rows;
		const unsigned n = B.rows;

		// Cache miss analysis: Inner Loop
		// A row-wise, B fixed, C column-wise (many cache misses!)
		for (unsigned matNo = 0; matNo < count; matNo++)
		{
			const double* a = A.start() + (matNo * l * m) % A.size();
			const double* b = B.start() + (matNo * m * n) % B.size();
			double* c = C->start() + (matNo * l * n);

			for (unsigned k = 0; k < n; k++)
			{
				const double* b_row = b + k * m;
				for (unsigned j = 0; j < m; j++)
				{
					const double* a_row = a + j * l;
					const double b_val = *(b_row + j);
					for (unsigned i = 0; i < l; i++)
					{
						*(c + (i * n + k)) += *(a_row + i) * b_val;
					}
				}
			}
		}
	}

	void __mat_matrix_mul_add(const mat_arr& A, const mat_arr& B, mat_arr* C, const mat_tr tr)
	{
#ifdef MAT_ARR_MATH_SIZE_CHECK
		const bool transpose_a = tr == transpose_A || tr == transpose_both;
		const bool transpose_b = tr == transpose_B || tr == transpose_both;
		__matrix_mul_size_check(A.count, transpose_a ? A.cols : A.rows, transpose_a ? A.rows : A.cols,
		                        B.count, transpose_b ? B.cols : B.rows, transpose_b ? B.rows : B.cols,
		                        C->count, C->rows, C->cols);

		if (A.start() == C->start() || B.start() == C->start())
		{
			throw runtime_error("Matrix mul in place not possible");
		}
#endif

		switch (tr)
		{
		case transpose_no:
			__mat_matrix_mul_add_case0(A, B, C);
			return;
		case transpose_A:
			__mat_matrix_mul_add_case1(A, B, C);
			return;
		case transpose_B:
			__mat_matrix_mul_add_case2(A, B, C);
			return;
		case transpose_both:
			__mat_matrix_mul_add_case3(A, B, C);
			return;
		}
	}

	void __mat_matrix_mul(const mat_arr& A, const mat_arr& B, mat_arr* C, const mat_tr tr)
	{
		mat_set_all(0, C);
		__mat_matrix_mul_add(A, B, C, tr);
	}
#pragma endregion

	mat_arr mat_matrix_mul_add(const mat_arr& A, const mat_arr& B, mat_arr* C, const mat_tr tr)
	{
		if (C == nullptr)
		{
			mat_arr tempC = mat_arr(max(A.count, B.count),
			                        (tr == transpose_A || tr == transpose_both) ? A.cols : A.rows,
			                        (tr == transpose_B || tr == transpose_both) ? B.rows : B.cols);
			__mat_matrix_mul_add(A, B, &tempC, tr);
			return tempC;
		}
		__mat_matrix_mul_add(A, B, C, tr);
		return *C;
	}

	mat_arr mat_matrix_mul(const mat_arr& A, const mat_arr& B, mat_arr* C, const mat_tr tr)
	{
		if (C == nullptr)
		{
			mat_arr tempC = mat_arr(max(A.count, B.count),
			                        (tr == transpose_A || tr == transpose_both) ? A.cols : A.rows,
			                        (tr == transpose_B || tr == transpose_both) ? B.rows : B.cols);
			__mat_matrix_mul(A, B, &tempC, tr);
			return tempC;
		}
		__mat_matrix_mul(A, B, C, tr);
		return *C;
	}

#pragma region transpose_internal
	void __transpose_size_check(const unsigned count_a, const unsigned rows_a, const unsigned cols_a,
	                            const unsigned count_c, const unsigned rows_c, const unsigned cols_c)
	{
		if (count_a != count_c)
		{
			throw runtime_error("Wrong array sizes");
		}

		if (rows_a != cols_c || cols_a != rows_c)
		{
			throw runtime_error("Wrong matrix dimensions");
		}
	}

	void __mat_transpose(const mat_arr& A, mat_arr* C)
	{
#ifdef MAT_ARR_MATH_SIZE_CHECK
		__transpose_size_check(A.count, A.rows, A.cols,
		                       C->count, C->rows, C->cols);
#endif

		const double* a = A.start();
		double* c = C->start();

		const unsigned rows = C->rows;
		const unsigned rows_count = C->count * C->rows;
		const unsigned cols = C->cols;

		int i_normal = 0;
		int i_transposed = 0;
		for (unsigned i = 0; i < rows_count; ++i)
		{
			for (unsigned col = 0; col < cols; ++col)
			{
				*(c + i_normal) = *(a + i_transposed);
				i_normal++;
				i_transposed += rows;
			}
			i_transposed -= (cols * rows - 1);
		}
	}
#pragma endregion

	mat_arr mat_transpose(const mat_arr& A, mat_arr* C)
	{
		if (C == nullptr)
		{
			mat_arr tempC = mat_arr(A.count, A.cols, A.rows);
			__mat_transpose(A, C);
			return tempC;
		}
		__mat_transpose(A, C);
		return *C;
	}

	mat_arr mat_set_all(double val, mat_arr* C)
	{
		if (C == nullptr)
		{
			throw runtime_error("C is nullptr");
		}
		const unsigned size = C->size();
		double* c = C->start();
		for (unsigned i = 0; i < size; i++)
		{
			*(c + i) = val;
		}
		return *C;
	}

	void __mat_concat_size_check(const vector<mat_arr>& mats, mat_arr* C)
	{
		const auto mats_count = static_cast<unsigned>(mats.size());
		unsigned count_sum = 0;
		for (unsigned i = 0; i < mats_count; i++)
		{
			if (C->rows != mats[i].rows)
			{
				throw runtime_error("Row count does not fit");
			}

			if (C->cols != mats[i].cols)
			{
				throw runtime_error("Column count does not fit");
			}

			count_sum += mats[i].count;
		}

		if (C->count != count_sum)
		{
			throw runtime_error("Array sizes do not fit");
		}
	}

	void __mat_concat_mats(const vector<mat_arr>& mats, mat_arr* C)
	{
#ifdef MAT_ARR_MATH_SIZE_CHECK
		__mat_concat_size_check(mats, C);
#endif
		const auto mat_arr_count = static_cast<unsigned>(mats.size());
		const unsigned row_col = C->rows * C->cols;
		double* c = C->start();

		for (unsigned mat_arr_no = 0; mat_arr_no < mat_arr_count; mat_arr_no++)
		{
			const unsigned size = row_col * mats[mat_arr_no].count;
			const double* m = mats[mat_arr_no].start();

			for (unsigned i = 0; i < size; i++)
			{
				*c = *m;
				c++;
				m++;
			}
		}
	}

	mat_arr mat_concat_mats(const vector<mat_arr>& mats, mat_arr* C)
	{
		if (C == nullptr)
		{
			unsigned count = 0;
			for (const auto& mat : mats)
			{
				count += mat.count;
			}
			mat_arr tempC = mat_arr(count, mats[0].rows, mats[0].cols);
			__mat_concat_mats(mats, &tempC);
			return tempC;
		}
		__mat_concat_mats(mats, C);
		return *C;
	}

	void __mat_select_mats_size_check(const mat_arr& A, const vector<unsigned>& indices, mat_arr* C)
	{
		if (C->rows != A.rows)
		{
			throw runtime_error("Row count does not fit");
		}

		if (C->cols != A.cols)
		{
			throw runtime_error("Column count does not fit");
		}

		if (static_cast<unsigned>(indices.size()) != C->count)
		{
			throw runtime_error("Array sizes do not fit");
		}
	}

	void __mat_select_mats(const mat_arr& A, const vector<unsigned>& indices, mat_arr* C)
	{
#ifdef MAT_ARR_MATH_SIZE_CHECK
		__mat_select_mats_size_check(A, indices, C);
#endif

		const auto mat_count = static_cast<unsigned>(indices.size());
		const unsigned row_col = C->rows * C->cols;
		double* c = C->start();
		for (unsigned mat_no = 0; mat_no < mat_count; mat_no++)
		{
			const double* a = A.start() + indices[mat_no] * row_col;
			for (unsigned rc = 0; rc < row_col; rc++)
			{
				*c = *a;
				a++;
				c++;
			}
		}
	}

	mat_arr mat_select_mats(const mat_arr& A, const vector<unsigned>& indices, mat_arr* C)
	{
		if (C == nullptr)
		{
			mat_arr tempC = mat_arr(static_cast<unsigned>(indices.size()), A.rows, A.cols);
			__mat_select_mats(A, indices, &tempC);
			return tempC;
		}
		__mat_select_mats(A, indices, C);
		return *C;
	}
}
