#include "qft.h"
#include "system_operations.h"

namespace qram_simulator
{
	QFT::QFT(std::string_view reg_in)
	{
		id = System::get(reg_in);
		n_digits = System::size_of(reg_in);
		double theta = 2 * pi / pow2(n_digits);
		omega = complex_t{ cos(theta), sin(theta) };
	}

	QFT::QFT(size_t reg_in)
	{
		id = reg_in;
		n_digits = System::size_of(reg_in);
		double theta = 2 * pi / pow2(n_digits);
		omega = complex_t{ cos(theta), sin(theta) };
	}

	/*
	about qft matrix

	< x | H | y >
	if ( x[i] & y[i] ) H = exp(2 * pi * i * x * y / 2^n) / 2^n
	else H = 1
	*/
	void QFT::operate(size_t l, size_t r, std::vector<System>& state) const
	{
		size_t n = r - l;
		size_t full_size = pow2(n_digits);
		size_t original_size = state.size();
		double extra_amplitude = 1.0 / std::sqrt(full_size);
		if (n == 0) return;
		else if (n == 1)
		{
			size_t v = val(l, state);
			state[l].amplitude *= extra_amplitude;
			state.insert(state.end(), full_size, state[l]);
			for (size_t i = 0; i < full_size; ++i)
			{
				size_t real_position = original_size + i;
				val(real_position, state) = i;
				state[real_position].amplitude *= std::pow(omega, i * v);
			}
			state[l].amplitude = 0;
		}
		else if (n == full_size)
		{
			std::vector<std::complex<double>> new_amps(full_size, 0);
			for (size_t i = 0; i < full_size; ++i)
			{
				size_t real_position = l + i;
				for (size_t j = 0; j < full_size; ++j)
				{
					new_amps[j] += state[real_position].amplitude * std::pow(omega, i * j);
				}
			}
			for (size_t i = 0; i < full_size; ++i)
			{
				state[i + l].amplitude = new_amps[i] * extra_amplitude;
			}
		}
		else // the general case
		{
			state.insert(state.end(), full_size, state[l]);
			std::vector<std::complex<double>> new_amps(full_size, 0);
			for (size_t i = l; i < r; ++i)
			{
				for (size_t j = 0; j < full_size; ++j)
				{
					size_t v = val(i, state);
					new_amps[j] += state[i].amplitude * std::pow(omega, v * j);
				}
			}
			for (size_t i = 0; i < full_size; ++i)
			{
				state[i + original_size].amplitude = new_amps[i] * extra_amplitude;
				val(i + original_size, state) = i;
			}
			for (size_t i = l; i < r; ++i)
				state[i].amplitude = 0;
		}
	}

	void QFT::operator()(std::vector<System>& state) const
	{
		profiler _("QFT");
		(SortExceptKey(id))(state);
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
			if (!compare_equal(state[iter_l], state[iter_r], id))
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


	inverseQFT::inverseQFT(std::string_view reg_in)
	{
		id = System::get(reg_in);
		n_digits = System::size_of(reg_in);
		double theta = 2 * pi / pow2(n_digits);
		omega = complex_t{ cos(theta), -sin(theta) };
	}

	inverseQFT::inverseQFT(size_t reg_in)
	{
		id = reg_in;
		n_digits = System::size_of(reg_in);
		double theta = 2 * pi / pow2(n_digits);
		omega = complex_t{ cos(theta), -sin(theta) };
	}

	/*
	about qft matrix

	< x | H | y >
	if ( x[i] & y[i] ) H = exp(2 * pi * i * x * y / 2^n) / 2^n
	else H = 1
	*/
	void inverseQFT::operate(size_t l, size_t r, std::vector<System>& state) const
	{
		size_t n = r - l;
		size_t full_size = pow2(n_digits);
		size_t original_size = state.size();
		double extra_amplitude = 1.0 / std::sqrt(full_size);
		if (n == 0) return;
		else if (n == 1)
		{
			size_t v = val(l, state);
			state[l].amplitude *= extra_amplitude;
			state.insert(state.end(), full_size, state[l]);
			for (size_t i = 0; i < full_size; ++i)
			{
				size_t real_position = original_size + i;
				val(real_position, state) = val(real_position, state) + i;
				state[real_position].amplitude *= std::pow(omega, i * v);
			}
			state[l].amplitude = 0;
		}
		else if (n == full_size)
		{
			std::vector<std::complex<double>> new_amps(full_size, 0);
			for (size_t i = 0; i < full_size; ++i)
			{
				size_t real_position = l + i;
				for (size_t j = 0; j < full_size; ++j)
				{
					new_amps[j] += state[real_position].amplitude * std::pow(omega, i * j);
				}
			}
			for (size_t i = 0; i < full_size; ++i)
			{
				state[i + l].amplitude = new_amps[i] * extra_amplitude;
			}
		}
		else // the general case
		{
			state.insert(state.end(), full_size, state[l]);
			std::vector<std::complex<double>> new_amps(full_size, 0);
			for (size_t i = l; i < r; ++i)
			{
				size_t v = val(i, state);
				for (size_t j = 0; j < full_size; ++j)
				{
					new_amps[j] += state[i].amplitude * std::pow(omega, v * j);
				}
			}
			for (size_t i = 0; i < full_size; ++i)
			{
				state[i + original_size].amplitude = new_amps[i] * extra_amplitude;
				val(i + original_size, state) = i;
			}
			for (size_t i = l; i < r; ++i)
				state[i].amplitude = 0;
		}
	}

	void inverseQFT::operator()(std::vector<System>& state) const
	{
		profiler _("inverseQFT");
		/*(MoveBackRegister(register_name))(state);
		SortUnconditional()(state);*/
		(SortExceptKey(id))(state);
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
			if (!compare_equal(state[iter_l], state[iter_r], id))
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

	struct QFTBucket
	{
		size_t count;
		std::vector<size_t> all_positions;

#ifndef SAFE_HASH
		size_t first_position;
#endif

		QFTBucket(size_t first_pos, size_t full_size)
			: count(0), all_positions(full_size, SIZE_MAX)
#ifndef SAFE_HASH
			, first_position(first_pos)
#endif
		{}
	};

	void QFT_Full::operate_bucket_sparse(const std::vector<size_t>& positions, std::vector<System>& state) const
	{
		double extra_amplitude = 1.0 / std::sqrt(full_size);
		size_t original_size = state.size(); // before insert
		bool inserted = false; // for lazy insertion

		for (size_t i = 0; i < positions.size(); ++i)
		{
			if (positions[i] == SIZE_MAX) continue;

			size_t v = val(positions[i], state);
			if (!inserted)
			{
				// insert at the first valid position
				state.insert(state.end(), full_size, state[positions[i]]);

				for (size_t j = 0; j < full_size; ++j)
				{
					size_t new_position = original_size + j;
					val(new_position, state) = j;
					state[new_position].amplitude = 0;
				}
				inserted = true;
			}

			// do matrix multiplication
			for (size_t j = 0; j < full_size; ++j)
			{
				size_t new_position = original_size + j;
				state[new_position].amplitude += (state[positions[i]].amplitude * std::pow(omega, (j * v)) * extra_amplitude);
			}

			// clear amplitude
			state[positions[i]].amplitude = 0;
		}
	}

	void QFT_Full::fft(const std::vector<size_t>& positions, std::vector<System>& state, bool inverse) const
	{
		/* Assign amplitudes to a temporary array */
		std::vector<complex_t> amps(positions.size(), 0);
		for (size_t i = 0; i < positions.size(); ++i)
		{
			amps[i] = state[positions[i]].amplitude;
		}

#ifndef SINGLE_THREAD
#pragma omp parallel for schedule(static)
#endif
		for (int64_t i = 0; i < full_size; ++i) {
			if (i < bitrev[i]) {
				std::swap(amps[i], amps[bitrev[i]]);
			}
		}

		std::vector<complex_t> W(full_size / 2);

		if (inverse)
		{
#ifndef SINGLE_THREAD
#pragma omp parallel for schedule(static)
#endif
			for (int64_t k = 0; k < full_size / 2; ++k) {
				profiler _("precompute wk");
				W[k] = std::exp(2 * pi * k / full_size * complex_t(0, 1));
			}
		}
		else
		{
#ifndef SINGLE_THREAD
#pragma omp parallel for schedule(static)
#endif
			for (int64_t k = 0; k < full_size / 2; ++k) {
				profiler _("precompute wk");
				W[k] = std::exp(-2 * pi * k / full_size * complex_t(0, 1));
			}
		}

		// 分阶段并行计算
		for (size_t s = 0; s < n_digits; ++s) {
			profiler _("fft");
			const size_t m = pow2(s + 1);
			const size_t m2 = m >> 1;
			const size_t w_step = full_size >> (s + 1);

			// 动态调整并行粒度
			const size_t chunk_size = (s < n_digits / 2)
				? (full_size / (m << 3))
				: (full_size / (m << 2));

#ifndef SINGLE_THREAD
#pragma omp parallel for schedule(dynamic, chunk_size)
#endif
			for (int64_t k = 0; k < full_size; k += m) {
				// 局部变量避免伪共享
				complex_t u, t;
				uint64_t idx1, idx2;

				for (size_t j = 0; j < m2; ++j) {
					const complex_t& w = W[j * w_step];
					idx1 = k + j;
					idx2 = k + j + m2;

					u = amps[idx1];
					t = amps[idx2] * w;  // SIMD可能自动向量化

					amps[idx1] = u + t;
					amps[idx2] = u - t;
				}
			}
		}
		double extra_amplitude = 1.0 / std::sqrt(full_size);
#ifndef SINGLE_THREAD
#pragma omp parallel for
#endif
		for (int64_t i = 0; i < full_size; ++i) {
			profiler _("normalize");
			state[positions[i]].amplitude = (amps[i] * extra_amplitude);
		}

	}

	void QFT_Full::operate_bucket_inplace(const std::vector<size_t>& positions, std::vector<System>& state) const
	{
		fft(positions, state, true);
	}


	void QFT_Full::operate_bucket_inplace_inv(const std::vector<size_t>& positions, std::vector<System>& state) const
	{
		fft(positions, state, false);
	}

	void QFT_Full::operator()(std::vector<System>& state) const
	{
		profiler _("QFT_Full");
		if (!state.size()) return;

		size_t current_size = state.size();

#ifdef SAFE_HASH
		StateLessExceptKey pred(id);
		//std::map<System, HadamardBucket, StateLessExceptKey> buckets(pred);
		std::map<System, std::pair<size_t, std::vector<size_t>>, StateLessExceptKey> buckets(pred);
#else
		auto hash_func = StateHashExceptKey(id);
		for (auto& s : state)
			s.cached_hash = hash_func(s);

		std::unordered_map<size_t, HadamardBucket> buckets;
#endif

		for (size_t i = 0; i < current_size; ++i)
		{
			if (!ConditionSatisfied(state[i]))
				continue;
#ifdef SAFE_HASH
			const auto& s = state[i];
			size_t val = s.GetAs(id, uint64_t);
			auto&& [iter, flag] = buckets.insert({ s, {0, std::vector<size_t>(full_size, SIZE_MAX)} });
			iter->second.second[val] = i;
			iter->second.first += 1;
			if (iter->second.first == full_size)
			{
				operate_bucket_inplace(iter->second.second, state);
				buckets.erase(iter);
			}
#else
			size_t s = state[i].cached_hash;
			size_t val = state[i].GetAs(id, uint64_t);
			auto&& [iter, flag] = buckets.insert({ s, HadamardBucket(i, bucket_full_size) });
			iter->second.all_positions[val] = i;
			iter->second.count += 1;
			if (iter->second.count == bucket_full_size)
			{
				operate_bucket_inplace(iter->second.all_positions, state);
				buckets.erase(iter);
			}
#endif

		}

		/* Remaining buckets */
		for (auto& [s, bucket] : buckets)
		{
#ifdef SAFE_HASH
			auto& all_pos = bucket.second;
#else
			auto& all_pos = bucket.all_positions;
#endif
			/* Case 1: When there are only few states in the bucket */
			if (bucket.first <= few_threshold)
			{
				operate_bucket_sparse(all_pos, state);
				continue;
			}

			/* Case 2: For many states in the bucket */
			for (size_t i = 0; i < all_pos.size(); ++i)
			{
				if (all_pos[i] == SIZE_MAX)
				{
#ifdef SAFE_HASH
					state.push_back(s);
#else
					state.push_back(state[bucket.first_position]);
#endif
					state.back().get(id).value = i;
					state.back().amplitude = 0;
					all_pos[i] = state.size() - 1;
				}
			}
			operate_bucket_inplace(all_pos, state);
		}
		ClearZero()(state);
		System::update_max_size(state.size());
	}


	void QFT_Full::dag(std::vector<System>& state) const
	{
		profiler _("QFT_Full");

		if (!state.size()) return;

		size_t current_size = state.size();

#ifdef SAFE_HASH
		StateLessExceptKey pred(id);
		//std::map<System, HadamardBucket, StateLessExceptKey> buckets(pred);
		std::map<System, std::pair<size_t, std::vector<size_t>>, StateLessExceptKey> buckets(pred);
#else
		auto hash_func = StateHashExceptKey(id);
		for (auto& s : state)
			s.cached_hash = hash_func(s);

		std::unordered_map<size_t, HadamardBucket> buckets;
#endif

		for (size_t i = 0; i < current_size; ++i)
		{
			if (!ConditionSatisfied(state[i]))
				continue;
#ifdef SAFE_HASH
			const auto& s = state[i];
			size_t val = s.GetAs(id, uint64_t);
			auto&& [iter, flag] = buckets.insert({ s, {0, std::vector<size_t>(full_size, SIZE_MAX)} });
			iter->second.second[val] = i;
			iter->second.first += 1;
			if (iter->second.first == full_size)
			{
				operate_bucket_inplace_inv(iter->second.second, state);
				buckets.erase(iter);
			}
#else
			size_t s = state[i].cached_hash;
			size_t val = state[i].GetAs(id, uint64_t);
			auto&& [iter, flag] = buckets.insert({ s, HadamardBucket(i, bucket_full_size) });
			iter->second.all_positions[val] = i;
			iter->second.count += 1;
			if (iter->second.count == bucket_full_size)
			{
				operate_bucket_inplace(iter->second.all_positions, state);
				buckets.erase(iter);
			}
#endif

		}

		/* Remaining buckets */
		for (auto& [s, bucket] : buckets)
		{
#ifdef SAFE_HASH
			auto& all_pos = bucket.second;
#else 
			auto& all_pos = bucket.all_positions;
#endif
			/* Case 1: When there are only few states in the bucket */
			if (bucket.first <= few_threshold)
			{
				operate_bucket_sparse(all_pos, state);
				continue;
			}

			/* Case 2: For many states in the bucket */
			for (size_t i = 0; i < all_pos.size(); ++i)
			{
				if (all_pos[i] == SIZE_MAX)
				{
#ifdef SAFE_HASH
					state.push_back(s);
#else
					state.push_back(state[bucket.first_position]);
#endif
					state.back().get(id).value = i;
					state.back().amplitude = 0;
					all_pos[i] = state.size() - 1;
				}
			}
			operate_bucket_inplace_inv(all_pos, state);
		}
		ClearZero()(state);
		System::update_max_size(state.size());
	}

}