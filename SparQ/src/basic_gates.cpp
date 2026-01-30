#include "basic_gates.h"

namespace qram_simulator
{
	void Phase_Bool::operator()(std::vector<System>& state) const
	{
		complex_t phase = complex_t(std::cos(lambda), std::sin(lambda));

#ifdef SINGLE_THREAD
		for (auto& s : state)
		{
#else
#pragma omp parallel for
		for (int i = 0; i < state.size(); ++i)
		{
			auto& s = state[i];
#endif
			auto& reg = s.get(id);
			if (!ConditionSatisfied(s))
				continue;

			if (digit1(reg.value, digit)) {
				s.amplitude *= phase;
			}
		}
	}

	void Phase_Bool::dag(std::vector<System>& state) const
	{
		complex_t phase = complex_t(std::cos(lambda), -std::sin(lambda));

#ifdef SINGLE_THREAD
		for (auto& s : state)
		{
#else
#pragma omp parallel for
		for (int i = 0; i < state.size(); ++i)
		{
			auto& s = state[i];
#endif
			auto& reg = s.get(id);
			if (!ConditionSatisfied(s))
				continue;
			if (digit1(reg.value, digit)) {
				s.amplitude *= phase;
			}
		}
	}

	//void Phase_Bool::display() const
	//{
	//	complex_t phase = complex_t(std::cos(lambda), -std::sin(lambda));

	//	size_t n_ctrl = condition_variable_by_bit.size();
	//	uint64_t size = pow2(n_ctrl + 1);

	//	DenseMatrix<complex_t> matrix(pow2(n_ctrl + 1));
	//	for (size_t i = 0; i < size - 2; i++)
	//	{
	//		matrix(i, i) = 1.0;
	//	}
	//	matrix(size - 2, size - 2) = 1.0;
	//	matrix(size - 1, size - 1) = phase;
	//	fmt::print("Phase gate\n{}\n", matrix.to_string());
	//}


	void Rot_Bool::operate(size_t l, size_t r, std::vector<System>&state) const
	{
		size_t n = r - l;
		constexpr size_t full_size = 2;
		size_t original_size = state.size();

		if (n == 0) return;

		//_operate_general(l, r, state, mat);
		//return;

		if (_is_diagonal(mat))
		{
			_operate_diagonal(l, r, state, mat);
		}
		else if (_is_off_diagonal(mat))
		{
			_operate_off_diagonal(l, r, state, mat);
		}
		else
		{
			_operate_general(l, r, state, mat);
		}

	}

	bool Rot_Bool::_is_diagonal(const u22_t & data)
	{
		if (abs_sqr(data[1]) < epsilon &&
			abs_sqr(data[2]) < epsilon)
		{
			return true;
		}
		return false;
	}

	void Rot_Bool::_operate_diagonal(size_t l, size_t r,
		std::vector<System>&state, const u22_t & mat) const
	{
		// diagonal means that no new elements will be created
		// any operation can be handled in-place

		std::complex<double> a0 = mat[0];
		std::complex<double> a1 = mat[3];

		for (size_t i = l; i < r; ++i)
		{
			auto& s = state[i];
			if (!ConditionSatisfied(s))
				return;

			if (s.get(id).value & mask)
			{
				s.amplitude *= a1;
			}
			else
			{
				s.amplitude *= a0;
			}
		}
	}

	bool Rot_Bool::_is_off_diagonal(const u22_t & data)
	{
		if (abs_sqr(data[0]) < epsilon &&
			abs_sqr(data[3]) < epsilon)
		{
			return true;
		}
		return false;
	}

	void Rot_Bool::_operate_off_diagonal(size_t l, size_t r,
		std::vector<System>&state, const u22_t & mat) const
	{
		// diagonal means that no new elements will be created
		// any operation can be handled in-place
		// with changing of storage (flipping)

		std::complex<double> a0 = mat[2];
		std::complex<double> a1 = mat[1];

		for (size_t i = l; i < r; ++i)
		{
			auto& s = state[i];
			auto& reg = s.get(id);
			if (!ConditionSatisfied(s))
				return;
			if (reg.value & mask)
			{
				s.amplitude *= a1;
				reg.value ^= mask; // flip
			}
			else
			{
				s.amplitude *= a0;
				reg.value ^= mask; // flip
			}
		}
	}

	void Rot_Bool::_operate_general(size_t l, size_t r,
		std::vector<System>&state, const u22_t & mat) const
	{
		size_t n = r - l;
		if (n == 1) // an extra entry should be added
		{
			if (!ConditionSatisfied(state[l]))
				return;
			size_t new_pos = state.size();
			state.push_back(state[l]);

			bool v = (state[l].get(id).value & mask);

			// if the original is 0
			if (!v)
			{
				state[new_pos].get(id).value ^= mask;

				state[l].amplitude *= mat[0];		// where |0>
				state[new_pos].amplitude *= mat[2]; // where |1>
			}
			// if the original is 1
			else
			{
				state[new_pos].get(id).value ^= mask;

				state[new_pos].amplitude *= mat[1]; // where |0>
				state[l].amplitude *= mat[3];		// where |1>
			}
		}
		else // everything can be computed in place
		{
			if (!ConditionSatisfied(state[l]))
				return;
			bool is_zero = state[l].GetAs(id, size_t) & pow2(digit);
			if (!is_zero) {
				complex_t a = state[l + 0].amplitude;
				complex_t b = state[l + 1].amplitude;
				state[l + 0].amplitude = a * mat[0] + b * mat[1];
				state[l + 1].amplitude = a * mat[2] + b * mat[3];
			}
			else {
				complex_t a = state[l + 0].amplitude;
				complex_t b = state[l + 1].amplitude;
				state[l + 1].amplitude = b * mat[0] + a * mat[1];
				state[l + 0].amplitude = b * mat[2] + a * mat[3];
			}
		}
	}

	void Rot_Bool::operate_pair(size_t zero, size_t one, std::vector<System>&state) const
	{
		complex_t a = state[zero].amplitude;
		complex_t b = state[one].amplitude;

		state[zero].amplitude = a * mat[0] + b * mat[1];
		state[one].amplitude = a * mat[2] + b * mat[3];
	}

	void Rot_Bool::operate_alone_zero(size_t zero, std::vector<System>&state) const
	{
		state.push_back(state[zero]);
		state.back().get(id).value ^= mask;

		state[zero].amplitude *= mat[0];
		state.back().amplitude *= mat[2];
	}

	void Rot_Bool::operate_alone_one(size_t one, std::vector<System>&state) const
	{
		state.push_back(state[one]);
		state.back().get(id).value ^= mask;

		state.back().amplitude *= mat[1];
		state[one].amplitude *= mat[3];
	}

	void Rot_Bool::operator()(std::vector<System>&state) const
	{
		bool use_hash = false;
		if (!use_hash)
		{
			profiler _("Rot_Bool_v1");

			(SortExceptBit(id, digit))(state);
			//StatePrint()(state);
			size_t current_size = state.size();
			auto iter_l = 0;
			auto iter_r = 1;

			while (true)
			{
				if (iter_r == current_size)
				{
					operate(iter_l, iter_r, state);
					break;
				}
				if (!compare_equal_rot(state[iter_l], state[iter_r], id, ~mask))
				{
					operate(iter_l, iter_r, state);
					iter_l = iter_r;
					iter_r = iter_l + 1;
				}
				else
				{
					iter_r++;
				}
			}
			ClearZero()(state);
			System::update_max_size(state.size());
		}
		else {
			profiler _("Rot_Bool_v2");
			if (!state.size()) return;

			std::set<size_t> qubit_positions = { digit };
#ifdef SAFE_HASH
			StateLessExceptQubits pred(id, { digit });
			std::map<System, size_t, StateLessExceptQubits> buckets(pred);
#else
			auto hash_func = StateHashExceptQubits(id, qubit_positions);
			for (auto& s : state)
				s.cached_hash = hash_func(s);

			std::unordered_map<size_t, size_t> buckets;
#endif
			size_t current_size = state.size();

			for (size_t i = 0; i < current_size; ++i)
			{
				if (!ConditionSatisfied(state[i]))
					return;
#ifdef SAFE_HASH
				const auto& s = state[i];
#else
				const auto& s = state[i].cached_hash;
#endif
				auto iter = buckets.find(s);

				if (iter == buckets.end())
				{
					buckets.insert({ s, i });
					continue;
				}
				else
				{
#ifdef CHECK_HASH
					auto pred = StateEqualExceptQubits(id, qubit_positions);
					if (!pred(state[iter->second], state[i]))
						throw_general_runtime_error();
#endif

					if (state[iter->second].get(id).value & mask)
					{
						// stored 1 and now 0
						operate_pair(i, iter->second, state);
					}
					else
					{
						// stored 0 and now 1
						operate_pair(iter->second, i, state);
					}
					buckets.erase(iter);
				}
			}

			for (auto& stored_key : buckets)
			{
				// alone
				if (state[stored_key.second].get(id).value & mask)
				{
					operate_alone_one(stored_key.second, state);
				}
				else
				{
					operate_alone_zero(stored_key.second, state);
				}
			}

			ClearZero()(state);
			System::update_max_size(state.size());
		}
	}

	void Rot_Bool::dag(std::vector<System>& state) const
	{
		Rot_Bool(id, digit, mat.dagger())(state);		
	}

	//void Rot_Bool::display() const
	//{
	//	size_t n_ctrl = condition_variable_by_bit.size();
	//	uint64_t size = pow2(n_ctrl + 1);

	//	DenseMatrix<complex_t> matrix(pow2(n_ctrl + 1));
	//	for (size_t i = 0; i < size - 2; i++)
	//	{
	//		matrix(i, i) = 1.0;
	//	}
	//	matrix(size - 2, size - 2) = mat[0];
	//	matrix(size - 2, size - 1) = mat[1];
	//	matrix(size - 1, size - 2) = mat[2];
	//	matrix(size - 1, size - 1) = mat[3];
	//	fmt::print("Rotation gate\n{}\n", matrix.to_string());
	//}

	void Xgate_Bool::operator()(std::vector<System>& state) const
	{
		profiler _("Xgate_Bool");
#ifdef SINGLE_THREAD
		for (auto& s : state)
		{
#else
#pragma omp parallel for
		for (int i = 0; i < state.size(); ++i)
		{
			auto& s = state[i];
#endif
			if (ConditionNotSatisfied(s))
				continue;

			auto& reg = s.get(id);
			reg.value = flip_digit(reg.value, digit);
		}
	}

	//void Xgate_Bool::display() const
	//{
	//	size_t n_ctrl = condition_variable_by_bit.size();
	//	uint64_t size = pow2(n_ctrl + 1);

	//	DenseMatrix<complex_t> matrix(pow2(n_ctrl + 1));
	//	for (size_t i = 0; i < size - 2; i++)
	//	{
	//		matrix(i, i) = 1.0;
	//	}
	//	matrix(size - 2, size - 1) = 1.0;
	//	matrix(size - 1, size - 2) = 1.0;
	//	fmt::print("X gate\n{}\n", matrix.to_string());
	//}

	void Ygate_Bool::operator()(std::vector<System>& state) const
	{
		complex_t phase_pos = complex_t(0, 1);
		complex_t phase_neg = complex_t(0, -1);
#ifdef SINGLE_THREAD
		for (auto& s : state)
		{
#else
#pragma omp parallel for
		for (int i = 0; i < state.size(); ++i)
		{
			auto& s = state[i];
#endif
			if (!ConditionSatisfied(s))
				continue;

			auto& reg = s.get(id);
			reg.value = flip_digit(reg.value, digit);
			if (digit1(reg.value, digit))
				s.amplitude *= phase_pos;
			else
				s.amplitude *= phase_neg;
		}
	}


	//void Ygate_Bool::display() const
	//{
	//	size_t n_ctrl = condition_variable_by_bit.size();
	//	uint64_t size = pow2(n_ctrl + 1);

	//	DenseMatrix<complex_t> matrix(pow2(n_ctrl + 1));
	//	for (size_t i = 0; i < size - 2; i++)
	//	{
	//		matrix(i, i) = 1.0;
	//	}
	//	matrix(size - 2, size - 1) = complex_t(0, -1);
	//	matrix(size - 1, size - 2) = complex_t(0, 1);
	//	fmt::print("Y gate\n{}\n", matrix.to_string());
	//}

	DenseMatrix<complex_t> Ygate_Bool::extract_matrix()
	{
		size_t nqubit = System::size_of(id);

		DenseMatrix<complex_t> ret(pow2(nqubit));

		for (size_t i = 0; i < pow2(nqubit); i++)
		{
			std::vector<System> state;
			state.emplace_back();
			state.back().get(id).value = i;

			std::vector<complex_t> vec(pow2(nqubit), 0);
			operator()(state);
#ifdef SINGLE_THREAD
			for (auto& s : state)
			{
#else
#pragma omp parallel for
			for (int i = 0; i < state.size(); ++i)
			{
				auto& s = state[i];
#endif
				size_t _index = s.get(id).value;
				vec[_index] = s.amplitude;
			}
			for (size_t j = 0; j < pow2(nqubit); j++) {
				ret(j, i) = vec[j];
			}
		}
		return ret;
	}

	//void Zgate_Bool::display() const
	//{
	//	auto n_ctrl = condition_variable_by_bit.size();
	//	auto size = pow2(n_ctrl + 1);

	//	DenseMatrix<complex_t> matrix(pow2(n_ctrl + 1));
	//	for (size_t i = 0; i < size - 2; i++)
	//	{
	//		matrix(i, i) = 1.0;
	//	}
	//	matrix(size - 2, size - 2) = complex_t(1, 0);
	//	matrix(size - 1, size - 1) = complex_t(-1, 0);
	//	fmt::print("Z gate\n{}\n", matrix.to_string());
	//}

	//void Sgate_Bool::display() const
	//{
	//	auto n_ctrl = condition_variable_by_bit.size();
	//	auto size = pow2(n_ctrl + 1);

	//	DenseMatrix<complex_t> matrix(pow2(n_ctrl + 1));
	//	for (size_t i = 0; i < size - 2; i++)
	//	{
	//		matrix(i, i) = 1.0;
	//	}
	//	matrix(size - 2, size - 2) = complex_t(1, 0);
	//	matrix(size - 1, size - 1) = complex_t(0, 1);
	//	fmt::print("S gate\n{}\n", matrix.to_string());
	//}

	//void Tgate_Bool::display() const
	//{
	//	auto n_ctrl = condition_variable_by_bit.size();
	//	auto size = pow2(n_ctrl + 1);

	//	DenseMatrix<complex_t> matrix(pow2(n_ctrl + 1));
	//	for (size_t i = 0; i < size - 2; i++)
	//	{
	//		matrix(i, i) = 1.0;
	//	}
	//	matrix(size - 2, size - 2) = complex_t(1, 0);
	//	matrix(size - 1, size - 1) = complex_t(cos(pi / 4), sin(pi / 4));
	//	fmt::print("T gate\n{}\n", matrix.to_string());
	//}

	RXgate_Bool::RXgate_Bool(std::string_view reg_, size_t digit_, double angle_)
		: Rot_Bool(reg_, digit_, {})
	{
		// todo check if digit is valid
		mat = { complex_t(cos(angle_ / 2), 0), complex_t(0, -sin(angle_ / 2)),
			complex_t(0, -sin(angle_ / 2)), complex_t(cos(angle_ / 2), 0) };
		Rot_Bool::mat = mat;
	};

	RXgate_Bool::RXgate_Bool(size_t id_, size_t digit_, double angle_)
		: Rot_Bool(id_, digit_, {})
	{
		// todo check if digit is valid
		mat = { complex_t(cos(angle_ / 2), 0), complex_t(0, -sin(angle_ / 2)),
			complex_t(0, -sin(angle_ / 2)), complex_t(cos(angle_ / 2), 0) };
		Rot_Bool::mat = mat;
	};

	//void RXgate_Bool::display() const
	//{
	//	auto n_ctrl = condition_variable_by_bit.size();
	//	auto size = pow2(n_ctrl + 1);

	//	DenseMatrix<complex_t> matrix(pow2(n_ctrl + 1));
	//	for (size_t i = 0; i < size - 2; i++)
	//	{
	//		matrix(i, i) = 1.0;
	//	}
	//	matrix(size - 2, size - 2) = mat[0];
	//	matrix(size - 2, size - 1) = mat[1];
	//	matrix(size - 1, size - 2) = mat[2];
	//	matrix(size - 1, size - 1) = mat[3];
	//	fmt::print("RX gate\n{}\n", matrix.to_string());
	//}

	RYgate_Bool::RYgate_Bool(std::string_view reg_, size_t digit_, double angle_)
		: Rot_Bool(reg_, digit_, {})
	{
		// todo check if digit is valid
		mat = { complex_t(cos(angle_ / 2), 0), complex_t(-sin(angle_ / 2), 0),
			complex_t(sin(angle_ / 2), 0), complex_t(cos(angle_ / 2), 0) };
		Rot_Bool::mat = mat;
	};

	RYgate_Bool::RYgate_Bool(size_t id_, size_t digit_, double angle_)
		: Rot_Bool(id_, digit_, {})
	{
		// todo check if digit is valid
		mat = { complex_t(cos(angle_ / 2), 0), complex_t(-sin(angle_ / 2), 0),
			complex_t(sin(angle_ / 2), 0), complex_t(cos(angle_ / 2), 0) };
		Rot_Bool::mat = mat;
	};

	//void RYgate_Bool::display() const
	//{
	//	auto n_ctrl = condition_variable_by_bit.size();
	//	auto size = pow2(n_ctrl + 1);

	//	DenseMatrix<complex_t> matrix(pow2(n_ctrl + 1));
	//	for (size_t i = 0; i < size - 2; i++)
	//	{
	//		matrix(i, i) = 1.0;
	//	}
	//	matrix(size - 2, size - 2) = mat[0];
	//	matrix(size - 2, size - 1) = mat[1];
	//	matrix(size - 1, size - 2) = mat[2];
	//	matrix(size - 1, size - 1) = mat[3];
	//	fmt::print("RY gate\n{}\n", matrix.to_string());
	//}

	RZgate_Bool::RZgate_Bool(std::string_view reg_, size_t digit_, double angle_)
		: GateBase(reg_, digit_), angle(angle_)
	{
		size_t size = System::size_of(id);
		if (digit >= size)
			throw_invalid_input();
	}

	RZgate_Bool::RZgate_Bool(size_t id_, size_t digit_, double angle_)
		: GateBase(id_, digit_), angle(angle_)
	{
		size_t size = System::size_of(id);
		if (digit >= size)
			throw_invalid_input();
	}

	void RZgate_Bool::operator()(std::vector<System>& state) const
	{
		// Todo : check if condition_variable_by_bit is valid.
		// That is, check if the conditioned bit is the same as operation itself.
		complex_t phase_pos = complex_t(cos(angle / 2), sin(angle / 2));
		complex_t phase_neg = complex_t(cos(angle / 2), -sin(angle / 2));

#ifdef SINGLE_THREAD
		for (auto& s : state)
		{
#else
#pragma omp parallel for
		for (int i = 0; i < state.size(); ++i)
		{
			auto& s = state[i];
#endif
			auto& reg = s.get(id);
			if (!ConditionSatisfied(s))
				continue;
			if (digit1(reg.value, digit)) {
				s.amplitude *= phase_pos;
			}
			else if (digit0(reg.value, digit)) {
				s.amplitude *= phase_neg;
			}
			else {
				throw_bad_result();
			}
		}

	}

	//void RZgate_Bool::display() const
	//{
	//	auto n_ctrl = condition_variable_by_bit.size();
	//	auto size = pow2(n_ctrl + 1);

	//	DenseMatrix<complex_t> matrix(pow2(n_ctrl + 1));
	//	for (size_t i = 0; i < size - 2; i++)
	//	{
	//		matrix(i, i) = 1.0;
	//	}
	//	complex_t phase_pos = complex_t(cos(angle / 2), sin(angle / 2));
	//	complex_t phase_neg = complex_t(cos(angle / 2), -sin(angle / 2));
	//	matrix(size - 2, size - 2) = phase_neg;
	//	matrix(size - 1, size - 1) = phase_pos;
	//	fmt::print("RZ gate\n{}\n", matrix.to_string());
	//}

	SXgate_Bool::SXgate_Bool(std::string_view reg_, size_t digit_)
		: Rot_Bool(reg_, digit_, {})
	{
		// todo check if digit is valid
		mat = { complex_t(1, 1) / 2.0, complex_t(1, -1) / 2.0,
				complex_t(1, -1) / 2.0, complex_t(1, 1) / 2.0 };
		Rot_Bool::mat = mat;
	};

	SXgate_Bool::SXgate_Bool(size_t id_, size_t digit_)
		: Rot_Bool(id_, digit_, {})
	{
		// todo check if digit is valid
		mat = { complex_t(1, 1) / 2.0, complex_t(1, -1) / 2.0,
				complex_t(1, -1) / 2.0, complex_t(1, 1) / 2.0 };
		Rot_Bool::mat = mat;
	};

	//void SXgate_Bool::display() const
	//{
	//	auto n_ctrl = condition_variable_by_bit.size();
	//	auto size = pow2(n_ctrl + 1);

	//	DenseMatrix<complex_t> matrix(pow2(n_ctrl + 1));
	//	for (size_t i = 0; i < size - 2; i++)
	//	{
	//		matrix(i, i) = 1.0;
	//	}
	//	matrix(size - 2, size - 2) = mat[0];
	//	matrix(size - 2, size - 1) = mat[1];
	//	matrix(size - 1, size - 2) = mat[2];
	//	matrix(size - 1, size - 1) = mat[3];
	//	fmt::print("SX gate\n{}\n", matrix.to_string());
	//}

	U2gate_Bool::U2gate_Bool(std::string_view reg_, size_t digit_, double phi, double lambda)
		: Rot_Bool(reg_, digit_, {}), phi(phi), lambda(lambda)
	{
		// todo check if digit is valid
		mat = { complex_t(1, 0) / sqrt2, complex_t(-cos(lambda), -sin(lambda)) / sqrt2,
				complex_t(cos(phi), sin(phi)) / sqrt2, complex_t(cos(lambda + phi), sin(lambda + phi)) / sqrt2 };
		Rot_Bool::mat = mat;
	};

	U2gate_Bool::U2gate_Bool(size_t id_, size_t digit_, double phi, double lambda)
		: Rot_Bool(id_, digit_, {}), phi(phi), lambda(lambda)
	{
		// todo check if digit is valid
		mat = { complex_t(1, 0) / sqrt2, complex_t(-cos(lambda), -sin(lambda)) / sqrt2,
				complex_t(cos(phi), sin(phi)) / sqrt2, complex_t(cos(lambda + phi), sin(lambda + phi)) / sqrt2 };
		Rot_Bool::mat = mat;
	};

	//void U2gate_Bool::display() const
	//{
	//	auto n_ctrl = condition_variable_by_bit.size();
	//	auto size = pow2(n_ctrl + 1);

	//	DenseMatrix<complex_t> matrix(pow2(n_ctrl + 1));
	//	for (size_t i = 0; i < size - 2; i++)
	//	{
	//		matrix(i, i) = 1.0;
	//	}
	//	matrix(size - 2, size - 2) = mat[0];
	//	matrix(size - 2, size - 1) = mat[1];
	//	matrix(size - 1, size - 2) = mat[2];
	//	matrix(size - 1, size - 1) = mat[3];
	//	fmt::print("U2 gate\n{}\n", matrix.to_string());
	//}

	U3gate_Bool::U3gate_Bool(std::string_view reg_, size_t digit_, double theta, double phi, double lambda)
		: Rot_Bool(reg_, digit_, {}), theta(theta), phi(phi), lambda(lambda)
	{
		// todo check if digit is valid
		mat = { complex_t(cos(theta / 2), 0),
				complex_t(-cos(lambda) * sin(theta / 2), -sin(lambda) * sin(theta / 2)),
				complex_t(cos(phi) * sin(theta / 2), sin(phi) * sin(theta / 2)),
				complex_t(cos(lambda + phi) * cos(theta / 2), sin(lambda + phi) * cos(theta / 2)) };
	};

	U3gate_Bool::U3gate_Bool(size_t id_, size_t digit_, double theta, double phi, double lambda)
		: Rot_Bool(id_, digit_, {}), theta(theta), phi(phi), lambda(lambda)
	{
		// todo check if digit is valid
		mat = { complex_t(cos(theta / 2), 0),
				complex_t(-cos(lambda) * sin(theta / 2), -sin(lambda) * sin(theta / 2)),
				complex_t(cos(phi) * sin(theta / 2), sin(phi) * sin(theta / 2)),
				complex_t(cos(lambda + phi) * cos(theta / 2), sin(lambda + phi) * cos(theta / 2)) };
	};

	//void U3gate_Bool::display() const
	//{
	//	auto n_ctrl = condition_variable_by_bit.size();
	//	auto size = pow2(n_ctrl + 1);

	//	DenseMatrix<complex_t> matrix(pow2(n_ctrl + 1));
	//	for (size_t i = 0; i < size - 2; i++)
	//	{
	//		matrix(i, i) = 1.0;
	//	}
	//	matrix(size - 2, size - 2) = mat[0];
	//	matrix(size - 2, size - 1) = mat[1];
	//	matrix(size - 1, size - 2) = mat[2];
	//	matrix(size - 1, size - 1) = mat[3];
	//	fmt::print("U3 gate\n{}\n", matrix.to_string());
	//}
}