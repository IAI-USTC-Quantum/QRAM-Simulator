/**
 * @file condrot.h
 * @brief 条件旋转门操作定义
 * @details 实现基于条件的旋转操作，包括有理数条件旋转和通用函数条件旋转
 */

#pragma once
#include "quantum_interfere_basic.h"

namespace qram_simulator
{
	/**
	 * @brief 创建旋转函数（基于值和位数）
	 * @param value 输入值
	 * @param n_digit 位数
	 * @return 2x2 旋转矩阵
	 */
	HOST_DEVICE inline u22_t make_func(uint64_t value, size_t n_digit)
	{
		double theta = 0;
		if (n_digit == 64)
			theta = value * 1.0 / 2 / pow2(63);
		else
			theta = value * 1.0 / pow2(n_digit);

		theta *= (2 * pi);

		complex_t u00 = cos(theta),
			u01 = -sin(theta),
			u10 = sin(theta),
			u11 = cos(theta);

		return u22_t{ u00, u01, u10, u11 };
	}

	/**
	 * @brief 创建逆向旋转函数（基于值和位数）
	 * @param value 输入值
	 * @param n_digit 位数
	 * @return 2x2 逆向旋转矩阵
	 */
	HOST_DEVICE inline u22_t make_func_inv(uint64_t value, size_t n_digit)
	{
		double theta = 0;
		if (n_digit == 64)
			theta = value * 1.0 / 2 / pow2(63);
		else
			theta = value * 1.0 / pow2(n_digit);

		theta *= (2 * pi);

		complex_t u00 = cos(theta),
			u01 = sin(theta),
			u10 = -sin(theta),
			u11 = cos(theta);

		return u22_t{ u00, u01, u10, u11 };
	}

	/**
	 * @brief 有理数条件旋转门（单比特）
	 * @details 基于有理数输入寄存器的值对布尔输出寄存器进行条件旋转
	 */
	struct CondRot_Rational_Bool : BaseOperator {
		using BaseOperator::operator();
		using BaseOperator::dag;

		/** @brief 输入寄存器 ID */
		size_t register_in;

		/** @brief 输出寄存器 ID */
		size_t register_out;

		/**
		 * @brief 构造函数（名称版本）
		 * @param reg_in 输入寄存器名称
		 * @param reg_out 输出寄存器名称
		 * @throws 当类型不匹配时抛出异常
		 */
		CondRot_Rational_Bool(std::string_view reg_in, std::string_view reg_out)
			:register_in(System::get(reg_in)), register_out(System::get(reg_out))
		{
			/* Type check */
#ifndef QRAM_Release
			if (System::type_of(register_in) != Rational ||
				System::type_of(register_out) != Boolean)
				throw_invalid_input();
#endif
		}

		/**
		 * @brief 构造函数（ID 版本）
		 * @param reg_in 输入寄存器 ID
		 * @param reg_out 输出寄存器 ID
		 */
		CondRot_Rational_Bool(size_t reg_in, size_t reg_out)
			:register_in(reg_in), register_out(reg_out)
		{
			/* Type check */
#ifndef QRAM_Release
			if (System::type_of(register_in) != Rational ||
				System::type_of(register_out) != Boolean)
				throw_invalid_input();
#endif
		}

		/**
		 * @brief 应用条件旋转操作
		 * @param state 系统状态向量
		 */
		void operator()(std::vector<System>& state) const;

		/**
		 * @brief 应用 dagger 操作
		 * @param state 系统状态向量
		 */
		void dag(std::vector<System>& state) const;
	};

	/**
	 * @brief 通用条件旋转门（单比特）
	 * @details 基于通用函数对布尔输出寄存器进行条件旋转
	 * @tparam Callable 角度计算函数类型
	 */
	template<typename Callable = std::function<u22_t(uint64_t)>>
	struct CondRot_General_Bool : BaseOperator {
		using BaseOperator::operator();
		using BaseOperator::dag;

		/** @brief 输入寄存器 ID */
		size_t in_id;

		/** @brief 输出寄存器 ID */
		size_t out_id;

		/** @brief 角度计算函数 */
		Callable func;

		/**
		 * @brief 构造函数（名称版本）
		 * @param reg_in 输入寄存器名称
		 * @param reg_out 输出寄存器名称
		 * @param angle_function 角度计算函数
		 * @throws 当类型不匹配或输出寄存器大小不为1时抛出异常
		 */
		CondRot_General_Bool(std::string_view reg_in, std::string_view reg_out, Callable angle_function)
			: in_id(System::get(reg_in)), out_id(System::get(reg_out)), func(angle_function)
		{
			/* Type check */
#ifndef QRAM_Release
			if (System::type_of(reg_out) != Boolean)
				throw_invalid_input();

			if (System::size_of(out_id) != 1)
				throw_invalid_input("Hadamard_Bool: size of output register must be 1");
#endif
		}

		/**
		 * @brief 构造函数（ID 版本）
		 * @param reg_in 输入寄存器 ID
		 * @param reg_out 输出寄存器 ID
		 * @param angle_function 角度计算函数
		 */
		CondRot_General_Bool(size_t reg_in, size_t reg_out, Callable angle_function)
			: in_id(reg_in), out_id(reg_out), func(angle_function)
		{
			/* Type check */
#ifndef QRAM_Release
			if (System::type_of(reg_out) != Boolean)
				throw_invalid_input();

			if (System::size_of(out_id) != 1)
				throw_invalid_input("Hadamard_Bool: size of output register must be 1");
#endif
		}

		/**
		 * @brief 成对操作
		 * @param zero |0> 分支索引
		 * @param one |1> 分支索引
		 * @param state 系统状态向量
		 */
		void operate_pair(size_t zero, size_t one, std::vector<System>& state) const
		{
			StateStorage& storage = state[zero].get(in_id);
			uint64_t v = storage.as<uint64_t>(System::size_of(in_id));
			u22_t mat = func(v);

			complex_t a = state[zero].amplitude;
			complex_t b = state[one].amplitude;
			state[zero].amplitude = a * mat[0] + b * mat[1];
			state[one].amplitude = a * mat[2] + b * mat[3];
		}

		/**
		 * @brief 单独操作 |0> 分支
		 * @param zero |0> 分支索引
		 * @param state 系统状态向量
		 */
		void operate_alone_zero(size_t zero, std::vector<System>& state) const
		{
			StateStorage& storage = state[zero].get(in_id);
			uint64_t v = storage.as<uint64_t>(System::size_of(in_id));
			u22_t mat = func(v);

			state.push_back(state[zero]);
			state.back().get(out_id).value = 1;

			state[zero].amplitude *= mat[0];
			state.back().amplitude *= mat[2];
		}

		/**
		 * @brief 单独操作 |1> 分支
		 * @param one |1> 分支索引
		 * @param state 系统状态向量
		 */
		void operate_alone_one(size_t one, std::vector<System>& state) const
		{
			StateStorage& storage = state[one].get(in_id);
			uint64_t v = storage.as<uint64_t>(System::size_of(in_id));
			u22_t mat = func(v);

			state.push_back(state[one]);
			state.back().get(out_id).value = 0;

			state.back().amplitude *= mat[1];
			state[one].amplitude *= mat[3];
		}

		/**
		 * @brief 应用通用条件旋转操作（V2 实现）
		 * @param state 系统状态向量
		 */
		void operator()(std::vector<System>& state) const
		{
			profiler _("CondRot_General_Bool_v2");

			if (!state.size()) return;

#ifdef SAFE_HASH
			StateLessExceptKey pred(out_id);
			std::map<System, size_t, StateLessExceptKey> buckets(pred);
#else
			auto hash_func = StateHashExceptKey(out_id);
			for (auto& s : state)
				s.cached_hash = hash_func(s);

			std::unordered_map<size_t, size_t> buckets;
#endif
			size_t current_size = state.size();

			for (size_t i = 0; i < current_size; ++i)
			{
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
					auto pred = StateEqualExceptKey(out_id);
					if (!pred(state[iter->second], state[i]))
						throw_general_runtime_error();
#endif
					StateStorage& storage = state[iter->second].get(out_id);
					if (storage.as<bool>(1))
					{
						operate_pair(i, iter->second, state);
					}
					else
					{
						operate_pair(iter->second, i, state);
					}
					buckets.erase(iter);
				}
			}

			for (auto& stored_key : buckets)
			{
				StateStorage& storage = state[stored_key.second].get(out_id);
				if (storage.as<bool>(1))
				{
					operate_alone_one(stored_key.second, state);
				}
				else
				{
					operate_alone_zero(stored_key.second, state);
				}
			}
			ClearZero()(state);
		}

#ifdef USE_CUDA
		/**
		 * @brief CUDA 应用通用条件旋转操作
		 * @param s CUDA 稀疏状态
		 */
		void operator()(CuSparseState& s) const;
#endif
	};

}
