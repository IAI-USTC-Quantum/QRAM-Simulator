/**
 * @file quantum_arithmetic.h
 * @brief 量子算术运算定义
 * @details 实现各种量子算术操作，包括翻转、移位、乘法、加法、比较等运算
 *
 * @section unitary_notes Unitary性质说明
 *
 * 量子算子必须满足unitary性质（U^†U = I），这要求操作是可逆的。本文件中的算子分为两类：
 *
 * 1. Out-of-place操作（如Add_UInt_UInt）：
 *    - 结果存储在独立的输出寄存器中
 *    - 通过bitwise XOR保证unitary：result ^= f(inputs)
 *    - 自动满足unitary，因为XOR是自逆操作
 *
 * 2. In-place操作（如Add_UInt_UInt_InPlace、Add_ConstUInt）：
 *    - 结果直接修改输入寄存器
 *    - 需要显式实现dagger()方法来保证可逆性
 *    - 通常使用模运算实现逆操作：y = (y + (2^N - x)) % 2^N
 *
 * @section type_safety_notes 类型安全说明
 *
 * 所有算子在debug模式（非QRAM_Release）下会检查：
 * - 输入/输出寄存器的类型（UnsignedInteger/SignedInteger/Boolean/Rational）
 * - 寄存器大小是否匹配
 * - 操作数的有效范围（如移位位数）
 *
 * Release模式下这些检查被编译移除，以获得最佳性能。
 */

#pragma once
#include "basic_components.h"

namespace qram_simulator
{
	/** @namespace qram_simulator
	 * @brief QRAM 稀疏态模拟器命名空间
	 */

	/**
	 * @brief 布尔翻转操作
	 * @details 翻转寄存器中的所有位（按位取反），实现 y = ~y
	 *
	 * @note Unitary性质：自伴算子（SelfAdjointOperator），即 U^† = U
	 * @note 数据类型：任意整数类型（UnsignedInteger/SignedInteger）
	 * @note 溢出行为：只影响寄存器size范围内的位，高位被翻转但会被mask截断
	 *
	 * @pre 输入寄存器必须是激活状态
	 *
	 * @par 示例
	 * @code
	 * // 4位寄存器，初始值 0b1010 (10)
	 * FlipBools("reg");  // 结果: 0b0101 (5)
	 * @endcode
	 */
	struct FlipBools : SelfAdjointOperator {
		using SelfAdjointOperator::operator();
		using SelfAdjointOperator::dag;

		ClassControllable

		/** @brief 寄存器 ID */
		size_t id;

		/**
		 * @brief 构造函数（名称版本）
		 * @param reg 寄存器名称
		 */
		FlipBools(std::string_view reg)
			:id(System::get(reg)) { }

		/**
		 * @brief 构造函数（ID 版本）
		 * @param id_ 寄存器 ID
		 */
		FlipBools(size_t id_)
			:id(id_)
		{}

		/**
		 * @brief 应用翻转操作
		 * @param state 系统状态向量
		 */
		void operator()(std::vector<System>& state) const;

#ifdef USE_CUDA
		/**
		 * @brief CUDA 应用翻转操作
		 * @param state CUDA 稀疏状态
		 */
		void operator()(CuSparseState& state) const;
#endif
	};

	/**
	 * @brief 双比特交换操作
	 * @details 交换两个寄存器中指定位置的单个比特
	 *
	 * 实现：使用临时变量交换两个寄存器指定位的值
	 * 这是一个自伴操作，应用两次等于恒等操作
	 *
	 * @note Unitary性质：自伴算子，Swap^2 = I
	 * @note 数据类型：任意类型寄存器的布尔位
	 *
	 * @pre lhs和rhs寄存器必须是激活状态
	 * @pre digit1和digit2必须在各自寄存器的有效位范围内 [0, size)
	 *
	 * @par 示例
	 * @code
	 * // reg1 = 0b1010, reg2 = 0b0101
	 * Swap_Bool_Bool("reg1", 0, "reg2", 1);  // 交换reg1的第0位和reg2的第1位
	 * @endcode
	 */
	struct Swap_Bool_Bool : SelfAdjointOperator {
		using SelfAdjointOperator::operator();
		using SelfAdjointOperator::dag;

		/** @brief 左操作数寄存器 ID */
		size_t lhs;

		/** @brief 右操作数寄存器 ID */
		size_t rhs;

		/** @brief 左操作数位索引 */
		size_t digit1;

		/** @brief 右操作数位索引 */
		size_t digit2;

		ClassControllable

		/**
		 * @brief 构造函数（名称版本）
		 * @param reg1 第一个寄存器名称
		 * @param d1 第一个位索引
		 * @param reg2 第二个寄存器名称
		 * @param d2 第二个位索引
		 * @throws 当位索引超出寄存器大小时抛出异常
		 */
		Swap_Bool_Bool(std::string_view reg1, size_t d1,
			std::string_view reg2, size_t d2)
			: lhs(System::get(reg1)), rhs(System::get(reg2)),
			digit1(d1), digit2(d2)
		{
			/* Size check */
#ifndef QRAM_Release
			if (d1 >= System::size_of(lhs) || d2 >= System::size_of(rhs))
				throw_invalid_input();
#endif
		}

		/**
		 * @brief 构造函数（ID 版本）
		 * @param id1 第一个寄存器 ID
		 * @param d1 第一个位索引
		 * @param id2 第二个寄存器 ID
		 * @param d2 第二个位索引
		 * @throws 当位索引超出寄存器大小时抛出异常
		 */
		Swap_Bool_Bool(size_t id1, size_t d1,
			size_t id2, size_t d2) : lhs(id1), rhs(id2), digit1(d1), digit2(d2)
		{
			/* Size check */
#ifndef QRAM_Release
			if (d1 >= System::size_of(lhs) || d2 >= System::size_of(rhs))
				throw_invalid_input();
#endif
		}

		/**
		 * @brief 应用交换操作
		 * @param state 系统状态向量
		 */
		void operator()(std::vector<System>& state) const;

#ifdef USE_CUDA
		/**
		 * @brief CUDA 应用交换操作
		 * @param state CUDA 稀疏状态
		 */
		void operator()(CuSparseState& state) const;
#endif
	};

	/**
	 * @brief 循环左移操作
	 * @details 将寄存器值循环左移指定位数，溢出的高位循环到低位
	 *
	 * 数学定义：y = (y << digit) | (y >> (N - digit))，其中N是寄存器位数
	 *
	 * @note Unitary性质：循环移位是双射，保证unitary
	 * @note dagger操作：左移d位的dagger是右移d位（或左移N-d位）
	 * @note 数据类型：建议UnsignedInteger，也可用于SignedInteger
	 *
	 * @pre register_1必须是激活状态
	 * @pre digit <= 寄存器大小（digit == size时等于恒等操作）
	 *
	 * @par 示例
	 * @code
	 * // 4位寄存器，初始值 0b1010
	 * ShiftLeft("reg", 1);  // 结果: 0b0101 (循环左移1位)
	 * ShiftLeft("reg", 2);  // 结果: 0b1010 (循环左移2位)
	 * @endcode
	 */
	struct ShiftLeft : BaseOperator {
		using BaseOperator::operator();
		using BaseOperator::dag;

		/** @brief 寄存器 ID */
		size_t register_1;

		/** @brief 移位位数 */
		size_t digit;

		ClassControllable

		/**
		 * @brief 构造函数（名称版本）
		 * @param reg1 寄存器名称
		 * @param d 移位位数
		 * @throws 当寄存器类型不是整数类型时抛出异常
		 */
		ShiftLeft(std::string_view reg1, size_t d)
			:register_1(System::get(reg1)), digit(d)
		{
			/* Type check */
#ifndef QRAM_Release
			auto type = System::type_of(register_1);
			if (type != UnsignedInteger && type != SignedInteger)
				throw_invalid_input();
			if (d > System::size_of(register_1))
				throw_invalid_input();
#endif
		}

		/**
		 * @brief 构造函数（ID 版本）
		 * @param reg1 寄存器 ID
		 * @param d 移位位数
		 * @throws 当寄存器类型不是整数类型时抛出异常
		 */
		ShiftLeft(size_t reg1, size_t d)
			:register_1(reg1), digit(d)
		{
			/* Type check */
#ifndef QRAM_Release
			auto type = System::type_of(register_1);
			if (type != UnsignedInteger && type != SignedInteger)
				throw_invalid_input();
			if (d > System::size_of(register_1))
				throw_invalid_input();
#endif
		}

		/**
		 * @brief 应用左移操作
		 * @param state 系统状态向量
		 */
		void operator()(std::vector<System>& state) const;

#ifdef USE_CUDA
		/**
		 * @brief CUDA 应用左移操作
		 * @param state CUDA 稀疏状态
		 */
		void operator()(CuSparseState& state) const;
#endif
	};


	/**
	 * @brief 循环右移操作
	 * @details 将寄存器值循环右移指定位数，溢出的低位循环到高位
	 *
	 * 数学定义：y = (y >> digit) | (y << (N - digit))，其中N是寄存器位数
	 *
	 * @note Unitary性质：循环移位是双射，保证unitary
	 * @note dagger操作：右移d位的dagger是左移d位（或右移N-d位）
	 * @note 数据类型：建议UnsignedInteger，也可用于SignedInteger
	 *
	 * @pre register_1必须是激活状态
	 * @pre digit <= 寄存器大小（digit == size时等于恒等操作）
	 *
	 * @par 示例
	 * @code
	 * // 4位寄存器，初始值 0b1010
	 * ShiftRight("reg", 1);  // 结果: 0b0101 (循环右移1位)
	 * @endcode
	 */
	struct ShiftRight : BaseOperator {
		using BaseOperator::operator();
		using BaseOperator::dag;

		/** @brief 寄存器 ID */
		size_t register_1;

		/** @brief 移位位数 */
		size_t digit;

		ClassControllable

		/**
		 * @brief 构造函数（名称版本）
		 * @param reg1 寄存器名称
		 * @param d 移位位数
		 * @throws 当寄存器类型不是整数类型时抛出异常
		 */
		ShiftRight(std::string_view reg1, size_t d)
			:register_1(System::get(reg1)), digit(d)
		{
			/* Type check */
#ifndef QRAM_Release
			auto type = System::type_of(register_1);
			if (type != UnsignedInteger && type != SignedInteger)
				throw_invalid_input();
			if (d > System::size_of(register_1))
				throw_invalid_input();
#endif
		}

		/**
		 * @brief 构造函数（ID 版本）
		 * @param reg1 寄存器 ID
		 * @param d 移位位数
		 * @throws 当寄存器类型不是整数类型时抛出异常
		 */
		ShiftRight(size_t reg1, size_t d)
			:register_1(reg1), digit(d)
		{
			/* Type check */
#ifndef QRAM_Release
			auto type = System::type_of(register_1);
			if (type != UnsignedInteger && type != SignedInteger)
				throw_invalid_input();
			if (d > System::size_of(register_1))
				throw_invalid_input();
#endif
		}

		/**
		 * @brief 应用右移操作
		 * @param state 系统状态向量
		 */
		void operator()(std::vector<System>& state) const;

#ifdef USE_CUDA
		/**
		 * @brief CUDA 应用右移操作
		 * @param state CUDA 稀疏状态
		 */
		void operator()(CuSparseState& state) const;
#endif
	};

	/**
	 * @brief 无符号整数乘常量操作（Out-of-place）
	 * @details 实现 out-of-place 乘法：res ^= lhs * mult
	 *
	 * Unitary保证：通过XOR实现，res = res ⊕ (lhs * mult)
	 * 应用两次：res ⊕ (lhs * mult) ⊕ (lhs * mult) = res，即 U^2 = I
	 *
	 * @note Unitary性质：自伴算子（U^† = U），因为XOR是自逆操作
	 * @note 数据类型：输入和输出都必须是UnsignedInteger
	 * @note 溢出行为：乘法结果按输出寄存器大小截断
	 *
	 * @pre lhs和res寄存器必须是UnsignedInteger类型
	 * @pre 所有寄存器必须是激活状态
	 *
	 * @par 示例
	 * @code
	 * auto lhs = System::add_register("lhs", UnsignedInteger, 4);
	 * auto res = System::add_register("res", UnsignedInteger, 4);
	 * Init_Unsafe(lhs, 3);  // lhs = 3
	 * // res = 0 ⊕ (3 * 4) = 12
	 * Mult_UInt_ConstUInt("lhs", 4, "res");
	 * @endcode
	 */
	struct Mult_UInt_ConstUInt : SelfAdjointOperator {
		using SelfAdjointOperator::operator();
		using SelfAdjointOperator::dag;

		/** @brief 乘数（常量） */
		size_t mult_int;

		/** @brief 左操作数寄存器 ID */
		size_t lhs;

		/** @brief 结果寄存器 ID */
		size_t res;

		ClassControllable

		/**
		 * @brief 构造函数（名称版本）
		 * @param reg_in 输入寄存器名称
		 * @param mult 乘数常量
		 * @param reg_out 输出寄存器名称
		 */
		Mult_UInt_ConstUInt(std::string_view reg_in, size_t mult, std::string_view reg_out)
			: lhs(System::get(reg_in)), res(System::get(reg_out)), mult_int(mult)
		{
			/* Type check */
#ifndef QRAM_Release
			if (System::type_of(lhs) != UnsignedInteger ||
				System::type_of(res) != UnsignedInteger)
				throw_invalid_input();
#endif	
		}

		/**
		 * @brief 构造函数（ID 版本）
		 * @param reg_in 输入寄存器 ID
		 * @param mult 乘数常量
		 * @param reg_out 输出寄存器 ID
		 */
		Mult_UInt_ConstUInt(size_t reg_in, size_t mult, size_t reg_out)
			: lhs(reg_in), res(reg_out), mult_int(mult)
		{
			/* Type check */
#ifndef QRAM_Release
			if (System::type_of(lhs) != UnsignedInteger ||
				System::type_of(res) != UnsignedInteger)
				throw_invalid_input();
#endif	
		}

		/**
		 * @brief 应用乘法操作
		 * @param state 系统状态向量
		 */
		void operator()(std::vector<System>& state) const;

#ifdef USE_CUDA
		/**
		 * @brief CUDA 应用乘法操作
		 * @param state CUDA 稀疏状态
		 */
		void operator()(CuSparseState& state) const;
#endif
	};

	/**
	 * @brief 累加乘常量操作（In-place）
	 * @details 实现 in-place 累加乘法：res += lhs * mult
	 *
	 * 这是一个in-place操作，需要显式实现dagger来保证unitary。
	 * dagger实现：res += (2^N - lhs * mult) mod 2^N
	 *
	 * @note Unitary性质：通过模运算保证可逆性
	 * @note 数据类型：建议lhs和res都为UnsignedInteger
	 * @note 溢出行为：结果按res寄存器大小模2^N回绕
	 *
	 * @pre res寄存器必须有足够位数存储结果
	 * @pre 当mult * max(lhs)可能溢出时，dagger操作需要特别注意
	 *
	 * @warning 此操作的unitary性依赖于正确的dagger实现。如果mult * lhs
	 *          在应用和dagger时溢出行为不一致，可能导致非unitary。
	 *
	 * @par 示例
	 * @code
	 * auto lhs = System::add_register("lhs", UnsignedInteger, 4);
	 * auto res = System::add_register("res", UnsignedInteger, 4);
	 * Init_Unsafe(lhs, 2);  // lhs = 2
	 * Init_Unsafe(res, 3);  // res = 3
	 * // res = 3 + (2 * 4) = 11
	 * Add_Mult_UInt_ConstUInt("lhs", 4, "res");
	 * @endcode
	 */
	struct Add_Mult_UInt_ConstUInt : BaseOperator {
		using BaseOperator::operator();
		using BaseOperator::dag;

		/** @brief 乘数（常量） */
		size_t mult_int;

		/** @brief 左操作数寄存器 ID */
		size_t lhs;

		/** @brief 结果寄存器 ID */
		size_t res;

		ClassControllable

		/**
		 * @brief 构造函数（名称版本）
		 * @param reg_in 输入寄存器名称
		 * @param mult 乘数常量
		 * @param reg_out 输出寄存器名称（结果累加至此）
		 * @throws 当寄存器类型不是UnsignedInteger时抛出异常
		 */
		Add_Mult_UInt_ConstUInt(std::string_view reg_in, size_t mult, std::string_view reg_out)
			: mult_int(mult), lhs(System::get(reg_in)), res(System::get(reg_out))
		{
			/* Type check */
#ifndef QRAM_Release
			if (System::type_of(lhs) != UnsignedInteger ||
				System::type_of(res) != UnsignedInteger)
				throw_invalid_input();
#endif
		}

		/**
		 * @brief 构造函数（ID 版本）
		 * @param reg_in 输入寄存器 ID
		 * @param mult 乘数常量
		 * @param reg_out 输出寄存器 ID（结果累加至此）
		 * @throws 当寄存器类型不是UnsignedInteger时抛出异常
		 */
		Add_Mult_UInt_ConstUInt(size_t reg_in, size_t mult, size_t reg_out)
			: mult_int(mult), lhs(reg_in), res(reg_out)
		{
			/* Type check */
#ifndef QRAM_Release
			if (System::type_of(lhs) != UnsignedInteger ||
				System::type_of(res) != UnsignedInteger)
				throw_invalid_input();
#endif
		}

		/**
		 * @brief 应用累加乘法操作
		 * @param state 系统状态向量
		 */
		void operator()(std::vector<System>& state) const;

		/**
		 * @brief 应用 dagger 操作
		 * @param state 系统状态向量
		 */
		void dag(std::vector<System>& state) const;

#ifdef USE_CUDA
		/**
		 * @brief CUDA 应用累加乘法操作
		 * @param state CUDA 稀疏状态
		 */
		void operator()(CuSparseState& state) const;

		/**
		 * @brief CUDA 应用 dagger 操作
		 * @param state CUDA 稀疏状态
		 */
		void dag(CuSparseState& state) const;
#endif
	};

	/**
	 * @brief 无符号整数加法操作（Out-of-place）
	 * @details 实现 out-of-place 加法：res ^= lhs + rhs
	 *
	 * Unitary保证：通过XOR实现，res = res ⊕ (lhs + rhs)
	 * 应用两次：res ⊕ (lhs + rhs) ⊕ (lhs + rhs) = res，即 U^2 = I
	 *
	 * @note Unitary性质：自伴算子（U^† = U），因为XOR是自逆操作
	 * @note 数据类型：lhs、rhs、res都必须是UnsignedInteger
	 * @note 溢出行为：加法结果按输出寄存器大小截断后XOR
	 *
	 * @pre lhs、rhs、res寄存器必须是UnsignedInteger类型
	 * @pre 所有寄存器必须是激活状态
	 * @pre res寄存器初始值通常为0，但也可以是任意值（XOR语义）
	 *
	 * @par 示例
	 * @code
	 * auto lhs = System::add_register("lhs", UnsignedInteger, 4);
	 * auto rhs = System::add_register("rhs", UnsignedInteger, 4);
	 * auto res = System::add_register("res", UnsignedInteger, 4);
	 * Init_Unsafe(lhs, 3);  // lhs = 3
	 * Init_Unsafe(rhs, 5);  // rhs = 5
	 * // res = 0 ⊕ (3 + 5) = 8
	 * Add_UInt_UInt("lhs", "rhs", "res");
	 * @endcode
	 */
	struct Add_UInt_UInt : SelfAdjointOperator {
		using SelfAdjointOperator::operator();
		using SelfAdjointOperator::dag;

		/** @brief 左操作数寄存器 ID */
		size_t lhs;

		/** @brief 右操作数寄存器 ID */
		size_t rhs;

		/** @brief 结果寄存器 ID */
		size_t res;

		ClassControllable

		/**
		 * @brief 构造函数（名称版本）
		 * @param lhs_ 左操作数寄存器名称
		 * @param rhs_ 右操作数寄存器名称
		 * @param res_ 结果寄存器名称
		 */
		Add_UInt_UInt(std::string_view lhs_, std::string_view rhs_, std::string_view res_)
			: lhs(System::get(lhs_)), rhs(System::get(rhs_)), res(System::get(res_))
		{
			/* Type check */
#ifndef QRAM_Release
			if (System::type_of(lhs) != UnsignedInteger ||
				System::type_of(rhs) != UnsignedInteger ||
				System::type_of(res) != UnsignedInteger)
				throw_invalid_input();
#endif	
		}

		/**
		 * @brief 构造函数（ID 版本）
		 * @param lhs_ 左操作数寄存器 ID
		 * @param rhs_ 右操作数寄存器 ID
		 * @param res_ 结果寄存器 ID
		 */
		Add_UInt_UInt(size_t lhs_, size_t rhs_, size_t res_)
			: lhs(lhs_), rhs(rhs_), res(res_)
		{
			/* Type check */
#ifndef QRAM_Release
			if (System::type_of(lhs) != UnsignedInteger ||
				System::type_of(rhs) != UnsignedInteger ||
				System::type_of(res) != UnsignedInteger)
				throw_invalid_input();
#endif	
		}

		/**
		 * @brief 应用加法操作
		 * @param state 系统状态向量
		 */
		void operator()(std::vector<System>& state) const;

#ifdef USE_CUDA
		/**
		 * @brief CUDA 应用加法操作
		 * @param state CUDA 稀疏状态
		 */
		void operator()(CuSparseState& state) const;
#endif
	};

	/**
	 * @brief 原地无符号整数加法操作（In-place）
	 * @details 实现 in-place 加法：rhs += lhs
	 *
	 * 这是一个in-place操作，需要显式实现dagger来保证unitary。
	 * dagger实现：rhs += (2^N - lhs) mod 2^N，其中N是rhs寄存器位数
	 *
	 * @note Unitary性质：通过模运算加法保证双射性
	 * @note 数据类型：lhs和rhs都应该是UnsignedInteger
	 * @note 溢出行为：结果按rhs寄存器大小模2^N回绕
	 *
	 * @pre lhs和rhs寄存器应该是相同大小（推荐）
	 * @pre lhs和rhs必须是激活状态
	 *
	 * @warning 如果lhs和rhs大小不同，较小的值会被截断，可能导致非unitary行为
	 *
	 * @par 示例
	 * @code
	 * auto lhs = System::add_register("lhs", UnsignedInteger, 4);
	 * auto rhs = System::add_register("rhs", UnsignedInteger, 4);
	 * Init_Unsafe(lhs, 7);  // lhs = 7
	 * Init_Unsafe(rhs, 3);  // rhs = 3
	 * // rhs = 3 + 7 = 10
	 * Add_UInt_UInt_InPlace("lhs", "rhs");
	 * // dagger: rhs = 10 + (16 - 7) % 16 = 3 (恢复原值)
	 * op.dag(state);
	 * @endcode
	 */
	struct Add_UInt_UInt_InPlace : BaseOperator {
		using BaseOperator::operator();
		using BaseOperator::dag;

		/** @brief 左操作数寄存器 ID */
		size_t lhs;

		/** @brief 右操作数寄存器 ID */
		size_t rhs;

		ClassControllable

		/**
		 * @brief 构造函数（名称版本）
		 * @param lhs_ 左操作数寄存器名称（加数）
		 * @param rhs_ 右操作数寄存器名称（被加数，结果存储于此）
		 * @throws 当寄存器类型不是UnsignedInteger或lhs大小超过rhs时抛出异常
		 */
		Add_UInt_UInt_InPlace(std::string_view lhs_, std::string_view rhs_)
			:lhs(System::get(lhs_)), rhs(System::get(rhs_))
		{
			/* Type check */
#ifndef QRAM_Release
			if (System::type_of(lhs) != UnsignedInteger ||
				System::type_of(rhs) != UnsignedInteger)
				throw_invalid_input();
			/* Size check - lhs must not exceed rhs (zero-extended add is safe) */
			if (System::size_of(lhs) > System::size_of(rhs))
				throw_invalid_input();
#endif
		}

		/**
		 * @brief 构造函数（ID 版本）
		 * @param lhs_ 左操作数寄存器 ID（加数）
		 * @param rhs_ 右操作数寄存器 ID（被加数，结果存储于此）
		 * @throws 当寄存器类型不是UnsignedInteger或lhs大小超过rhs时抛出异常
		 */
		Add_UInt_UInt_InPlace(size_t lhs_, size_t rhs_)
			: lhs(lhs_), rhs(rhs_)
		{
			/* Type check */
#ifndef QRAM_Release
			if (System::type_of(lhs) != UnsignedInteger ||
				System::type_of(rhs) != UnsignedInteger)
				throw_invalid_input();
			/* Size check - lhs must not exceed rhs (zero-extended add is safe) */
			if (System::size_of(lhs) > System::size_of(rhs))
				throw_invalid_input();
#endif
		}

		/**
		 * @brief 应用原地加法操作
		 * @param state 系统状态向量
		 */
		void operator()(std::vector<System>& state) const;

		/**
		 * @brief 应用 dagger 操作
		 * @param state 系统状态向量
		 */
		void dag(std::vector<System>& state) const;

#ifdef USE_CUDA
		/**
		 * @brief CUDA 应用原地加法操作
		 * @param state CUDA 稀疏状态
		 */
		void operator()(CuSparseState& state) const;

		/**
		 * @brief CUDA 应用 dagger 操作
		 * @param state CUDA 稀疏状态
		 */
		void dag(CuSparseState& state) const;
#endif
	};

	/**
	 * @brief 无符号整数加常量操作（Out-of-place）
	 * @details 实现 out-of-place 加常量：res ^= lhs + add_int
	 *
	 * Unitary保证：通过XOR实现，res = res ⊕ (lhs + add_int)
	 * 应用两次：res ⊕ (lhs + add_int) ⊕ (lhs + add_int) = res
	 *
	 * @note Unitary性质：自伴算子（U^† = U），因为XOR是自逆操作
	 * @note 数据类型：lhs和res都必须是UnsignedInteger
	 * @note 溢出行为：加法结果按输出寄存器大小截断后XOR
	 *
	 * @pre lhs和res寄存器必须是UnsignedInteger类型
	 * @pre 所有寄存器必须是激活状态
	 *
	 * @par 示例
	 * @code
	 * auto lhs = System::add_register("lhs", UnsignedInteger, 4);
	 * auto res = System::add_register("res", UnsignedInteger, 4);
	 * Init_Unsafe(lhs, 6);  // lhs = 6
	 * // res = 0 ⊕ (6 + 4) = 10
	 * Add_UInt_ConstUInt("lhs", 4, "res");
	 * @endcode
	 */
	struct Add_UInt_ConstUInt : SelfAdjointOperator {
		using SelfAdjointOperator::operator();
		using SelfAdjointOperator::dag;

		/** @brief 加数（常量） */
		size_t add_int;

		/** @brief 左操作数寄存器 ID */
		size_t lhs;

		/** @brief 结果寄存器 ID */
		size_t res;

		ClassControllable

		/**
		 * @brief 构造函数（名称版本）
		 * @param reg_in 输入寄存器名称
		 * @param add 加数常量
		 * @param reg_out 输出寄存器名称
		 */
		Add_UInt_ConstUInt(std::string_view reg_in, size_t add, std::string_view reg_out)
			: lhs(System::get(reg_in)), res(System::get(reg_out)), add_int(add)
		{
			/* Type check */
#ifndef QRAM_Release
			if (System::type_of(lhs) != UnsignedInteger ||
				System::type_of(res) != UnsignedInteger)
				throw_invalid_input();
#endif		
		}

		/**
		 * @brief 构造函数（ID 版本）
		 * @param reg_in 输入寄存器 ID
		 * @param add 加数常量
		 * @param reg_out 输出寄存器 ID
		 */
		Add_UInt_ConstUInt(size_t reg_in, size_t add, size_t reg_out)
			: lhs(reg_in), res(reg_out), add_int(add)
		{
			/* Type check */
#ifndef QRAM_Release
			if (System::type_of(lhs) != UnsignedInteger ||
				System::type_of(res) != UnsignedInteger)
				throw_invalid_input();
#endif		
		}

		/**
		 * @brief 应用加常量操作
		 * @param state 系统状态向量
		 */
		void operator()(std::vector<System>& state) const;

#ifdef USE_CUDA
		/**
		 * @brief CUDA 应用加常量操作
		 * @param state CUDA 稀疏状态
		 */
		void operator()(CuSparseState& state) const;
#endif
	};

	/**
	 * @brief 加常量操作（In-place）
	 * @details 实现 in-place 加常量：reg_in += add_int (mod 2^N)
	 *
	 * 这是一个in-place操作，需要显式实现dagger来保证unitary。
	 * dagger实现：reg_in += (2^N - add_int) mod 2^N，其中N是寄存器位数
	 *
	 * @note Unitary性质：通过模运算加法保证双射性
	 * @note 数据类型：建议reg_in为UnsignedInteger
	 * @note 溢出行为：结果按寄存器大小模2^N回绕
	 *
	 * @pre reg_in必须是激活状态
	 * @pre add_int应该小于2^N（N为寄存器位数），否则行为取决于模运算
	 *
	 * @par 示例
	 * @code
	 * auto reg = System::add_register("reg", UnsignedInteger, 4);
	 * Init_Unsafe(reg, 12);  // reg = 12
	 * // reg = (12 + 3) % 16 = 15
	 * Add_ConstUInt("reg", 3);
	 * // dagger: reg = (15 + 13) % 16 = 12 (恢复原值)
	 * op.dag(state);
	 * @endcode
	 */
	struct Add_ConstUInt : BaseOperator {
		using BaseOperator::operator();
		using BaseOperator::dag;

		/** @brief 加数（常量） */
		size_t add_int;

		/** @brief 输入寄存器 ID */
		size_t reg_in;

		ClassControllable

		/**
		 * @brief 构造函数（名称版本）
		 * @param reg_in_ 输入寄存器名称（结果存储于此）
		 * @param add 加数常量
		 * @throws 当寄存器类型不是整数类型时抛出异常
		 */
		Add_ConstUInt(std::string_view reg_in_, size_t add) :
			reg_in(System::get(reg_in_)), add_int(add)
		{
			/* Type check */
#ifndef QRAM_Release
			auto type = System::type_of(reg_in);
			if (type != UnsignedInteger && type != SignedInteger)
				throw_invalid_input();
#endif
		}

		/**
		 * @brief 构造函数（ID 版本）
		 * @param reg_in 输入寄存器 ID（结果存储于此）
		 * @param add 加数常量
		 * @throws 当寄存器类型不是整数类型时抛出异常
		 */
		Add_ConstUInt(size_t reg_in, size_t add)
			: reg_in(reg_in), add_int(add)
		{
			/* Type check */
#ifndef QRAM_Release
			auto type = System::type_of(reg_in);
			if (type != UnsignedInteger && type != SignedInteger)
				throw_invalid_input();
#endif
		}

		/**
		 * @brief 应用加常量操作
		 * @param state 系统状态向量
		 */
		void operator()(std::vector<System>& state) const;

		/**
		 * @brief 应用 dagger 操作
		 * @param state 系统状态向量
		 */
		void dag(std::vector<System>& state) const;

#ifdef USE_CUDA
		/**
		 * @brief CUDA 应用加常量操作
		 * @param state CUDA 稀疏状态
		 */
		void operator()(CuSparseState& state) const;

		/**
		 * @brief CUDA 应用 dagger 操作
		 * @param state CUDA 稀疏状态
		 */
		void dag(CuSparseState& state) const;
#endif
	};

	/**
	 * @brief 除法平方根反余弦操作（Out-of-place）
	 * @details 实现：res ^= arccos(sqrt(lhs / rhs)) / π / 2
	 *
	 * 用于计算量子旋转角度，常见于量子机器学习算法中。
	 *
	 * @note Unitary性质：自伴算子，通过XOR实现
	 * @note 数据类型：lhs和rhs必须是UnsignedInteger，res必须是Rational
	 * @note 数值范围：结果在[0, 0.5]范围内，编码为有理数
	 *
	 * @pre lhs和rhs必须是UnsignedInteger类型
	 * @pre res必须是Rational类型
	 * @pre lhs < rhs（否则sqrt参数超出[0,1]范围，可能产生NaN）
	 * @pre 所有寄存器必须是激活状态
	 *
	 * @par 数学公式
	 * output = arccos(√(lhs / rhs)) / (2π)
	 *
	 * @par 示例
	 * @code
	 * auto lhs = System::add_register("lhs", UnsignedInteger, 4);
	 * auto rhs = System::add_register("rhs", UnsignedInteger, 4);
	 * auto res = System::add_register("res", Rational, 8);
	 * Init_Unsafe(lhs, 1);  // lhs = 1
	 * Init_Unsafe(rhs, 4);  // rhs = 4
	 * // res = arccos(sqrt(1/4)) / 2π = arccos(0.5) / 2π = 1/6 ≈ 0.167
	 * Div_Sqrt_Arccos_Int_Int("lhs", "rhs", "res");
	 * @endcode
	 */
	struct Div_Sqrt_Arccos_Int_Int : SelfAdjointOperator {
		using SelfAdjointOperator::operator();
		using SelfAdjointOperator::dag;

		/** @brief 左操作数寄存器 ID */
		size_t register_lhs;

		/** @brief 右操作数寄存器 ID */
		size_t register_rhs;

		/** @brief 输出寄存器 ID */
		size_t register_out;

		ClassControllable

		/**
		 * @brief 构造函数（名称版本）
		 * @param register_lhs 左操作数寄存器名称
		 * @param register_rhs 右操作数寄存器名称
		 * @param register_out 输出寄存器名称
		 */
		Div_Sqrt_Arccos_Int_Int(std::string_view register_lhs, std::string_view register_rhs, std::string_view register_out)
			:register_lhs(System::get(register_lhs)),
			register_rhs(System::get(register_rhs)),
			register_out(System::get(register_out))
		{
			/* Type check */
#ifndef QRAM_Release
		/*if (System::type_of(register_lhs) != UnsignedInteger ||
			System::type_of(register_rhs) != UnsignedInteger ||
			System::type_of(register_out) != Rational)
			throw_invalid_input();*/
#endif		
		}

		/**
		 * @brief 构造函数（ID 版本）
		 * @param reg_lhs 左操作数寄存器 ID
		 * @param reg_rhs 右操作数寄存器 ID
		 * @param reg_out 输出寄存器 ID
		 */
		Div_Sqrt_Arccos_Int_Int(size_t reg_lhs, size_t reg_rhs, size_t reg_out)
			:register_lhs(reg_lhs),
			register_rhs(reg_rhs),
			register_out(reg_out)
		{
			/* Type check */
#ifndef QRAM_Release
			/*if (System::type_of(register_lhs) != UnsignedInteger ||
				System::type_of(register_rhs) != UnsignedInteger ||
				System::type_of(register_out) != Rational)
				throw_invalid_input();*/
#endif		
		}

		/**
		 * @brief 应用运算操作
		 * @param state 系统状态向量
		 */
		void operator()(std::vector<System>& state) const;

#ifdef USE_CUDA
		/**
		 * @brief CUDA 应用运算操作
		 * @param state CUDA 稀疏状态
		 */
		void operator()(CuSparseState& state) const;
#endif
	};

	/**
	 * @brief 平方根除法反余弦操作（Out-of-place）
	 * @details 实现：res ^= arccos(lhs / sqrt(rhs)) / π / 2
	 *
	 * 用于计算量子旋转角度，常见于量子振幅编码。
	 *
	 * @note Unitary性质：自伴算子，通过XOR实现
	 * @note 数据类型：lhs必须是SignedInteger，rhs必须是UnsignedInteger，res必须是Rational
	 * @note 数值范围：结果在[0, 1)范围内，编码为有理数
	 *
	 * @pre lhs必须是SignedInteger类型
	 * @pre rhs必须是UnsignedInteger类型
	 * @pre res必须是Rational类型
	 * @pre |lhs| <= sqrt(rhs)（保证arccos参数在[-1,1]范围内）
	 * @pre 所有寄存器必须是激活状态
	 *
	 * @par 数学公式
	 * output = arccos(lhs / √rhs) / (2π)
	 *
	 * @par 示例
	 * @code
	 * auto lhs = System::add_register("lhs", SignedInteger, 4);
	 * auto rhs = System::add_register("rhs", UnsignedInteger, 4);
	 * auto res = System::add_register("res", Rational, 8);
	 * Init_Unsafe(lhs, 1);   // lhs = 1
	 * Init_Unsafe(rhs, 4);   // rhs = 4
	 * // res = arccos(1/2) / 2π = 1/6 ≈ 0.167
	 * Sqrt_Div_Arccos_Int_Int("lhs", "rhs", "res");
	 * @endcode
	 */
	struct Sqrt_Div_Arccos_Int_Int : SelfAdjointOperator {
		using SelfAdjointOperator::operator();
		using SelfAdjointOperator::dag;

		/** @brief 左操作数寄存器 ID */
		size_t register_lhs;

		/** @brief 右操作数寄存器 ID */
		size_t register_rhs;

		/** @brief 输出寄存器 ID */
		size_t register_out;

		ClassControllable

		/**
		 * @brief 构造函数（名称版本）
		 * @param lhs 左操作数寄存器名称
		 * @param rhs 右操作数寄存器名称
		 * @param out 输出寄存器名称
		 */
		Sqrt_Div_Arccos_Int_Int(std::string_view lhs, std::string_view rhs, std::string_view out)
			:register_lhs(System::get(lhs)),
			register_rhs(System::get(rhs)),
			register_out(System::get(out))
		{
			/* Type check */
#ifndef QRAM_Release
			if (System::type_of(register_lhs) != SignedInteger ||
				System::type_of(register_rhs) != UnsignedInteger ||
				System::type_of(register_out) != Rational)
				throw_invalid_input();
#endif
		}

		/**
		 * @brief 构造函数（ID 版本）
		 * @param reg_lhs 左操作数寄存器 ID
		 * @param reg_rhs 右操作数寄存器 ID
		 * @param reg_out 输出寄存器 ID
		 */
		Sqrt_Div_Arccos_Int_Int(size_t reg_lhs, size_t reg_rhs, size_t reg_out)
			:register_lhs(reg_lhs),
			register_rhs(reg_rhs),
			register_out(reg_out)
		{
			/* Type check */
#ifndef QRAM_Release
			if (System::type_of(register_lhs) != SignedInteger ||
				System::type_of(register_rhs) != UnsignedInteger ||
				System::type_of(register_out) != Rational)
				throw_invalid_input();
#endif
		}

		/**
		 * @brief 应用运算操作
		 * @param state 系统状态向量
		 */
		void operator()(std::vector<System>& state) const;

#ifdef USE_CUDA
		/**
		 * @brief CUDA 应用运算操作
		 * @param state CUDA 稀疏状态
		 */
		void operator()(CuSparseState& state) const;
#endif
	};


	/**
	 * @brief 获取旋转角度操作（Out-of-place）
	 * @details 计算从lhs到rhs的极坐标角度：res ^= atan2(rhs, lhs) / (2π)
	 *
	 * 将笛卡尔坐标(lhs, rhs)转换为极坐标角度，结果编码在[0, 1)范围内。
	 *
	 * @note Unitary性质：自伴算子，通过XOR实现
	 * @note 数据类型：lhs和rhs可以是SignedInteger或UnsignedInteger，res必须是Rational
	 * @note 数值范围：结果在[0, 1)范围内（归一化的角度）
	 *
	 * @pre res必须是Rational类型
	 * @pre 所有寄存器必须是激活状态
	 *
	 * @par 数学公式
	 * - 如果 lhs == 0 且 rhs >= 0: output = 0.25 (90°)
	 * - 如果 lhs == 0 且 rhs < 0: output = 0.75 (270°)
	 * - 其他: output = atan2(rhs, lhs) / (2π) 归一化到[0,1)
	 *
	 * @par 示例
	 * @code
	 * auto lhs = System::add_register("lhs", SignedInteger, 4);
	 * auto rhs = System::add_register("rhs", SignedInteger, 4);
	 * auto res = System::add_register("res", Rational, 8);
	 * Init_Unsafe(lhs, 1);  // lhs = 1
	 * Init_Unsafe(rhs, 1);  // rhs = 1
	 * // res = atan2(1, 1) / 2π = 0.125 (45°)
	 * GetRotateAngle_Int_Int("lhs", "rhs", "res");
	 * @endcode
	 */
	struct GetRotateAngle_Int_Int : SelfAdjointOperator {
		using SelfAdjointOperator::operator();
		using SelfAdjointOperator::dag;

		/** @brief 左操作数寄存器 ID */
		size_t register_lhs;

		/** @brief 右操作数寄存器 ID */
		size_t register_rhs;

		/** @brief 输出寄存器 ID */
		size_t register_out;

		ClassControllable

		/**
		 * @brief 构造函数（名称版本）
		 * @param reg_lhs 左操作数寄存器名称
		 * @param reg_rhs 右操作数寄存器名称
		 * @param reg_out 输出寄存器名称
		 */
		GetRotateAngle_Int_Int(std::string_view reg_lhs, std::string_view reg_rhs, std::string_view reg_out)
			:register_lhs(System::get(reg_lhs)),
			register_rhs(System::get(reg_rhs)),
			register_out(System::get(reg_out))
		{
		}

		/**
		 * @brief 构造函数（ID 版本）
		 * @param reg_lhs 左操作数寄存器 ID
		 * @param reg_rhs 右操作数寄存器 ID
		 * @param reg_out 输出寄存器 ID
		 */
		GetRotateAngle_Int_Int(size_t reg_lhs, size_t reg_rhs, size_t reg_out)
			:register_lhs(reg_lhs),
			register_rhs(reg_rhs),
			register_out(reg_out)
		{
		}

		/**
		 * @brief 应用运算操作
		 * @param state 系统状态向量
		 */
		void operator()(std::vector<System>& state) const;

#ifdef USE_CUDA
		/**
		 * @brief CUDA 应用运算操作
		 * @param state CUDA 稀疏状态
		 */
		void operator()(CuSparseState& state) const;
#endif
	};

	/**
	 * @brief 任意整数累加操作（In-place）
	 * @details 实现 in-place 加法：lhs += rhs，支持有符号和无符号整数
	 *
	 * 这是一个in-place操作，需要显式实现dagger来保证unitary。
	 * dagger实现：lhs -= rhs (mod 2^N)
	 *
	 * @note Unitary性质：通过模运算保证双射性
	 * @note 数据类型：lhs和rhs可以是UnsignedInteger或SignedInteger
	 * @note 溢出行为：结果按lhs寄存器大小模2^N回绕
	 * @note 类型组合：支持混合类型（如lhs为SignedInteger，rhs为UnsignedInteger）
	 *
	 * @pre lhs和rhs必须是整数类型（UnsignedInteger或SignedInteger）
	 * @pre 所有寄存器必须是激活状态
	 *
	 * @warning 有符号整数的溢出行为是未定义的（C++标准），使用时需谨慎
	 *
	 * @par 示例
	 * @code
	 * auto lhs = System::add_register("lhs", UnsignedInteger, 4);
	 * auto rhs = System::add_register("rhs", UnsignedInteger, 4);
	 * Init_Unsafe(lhs, 8);  // lhs = 8
	 * Init_Unsafe(rhs, 6);  // rhs = 6
	 * // lhs = (8 + 6) % 16 = 14
	 * AddAssign_AnyInt_AnyInt("lhs", "rhs");
	 * // dagger: lhs = (14 - 6) % 16 = 8 (恢复原值)
	 * op.dag(state);
	 * @endcode
	 */
	struct AddAssign_AnyInt_AnyInt : BaseOperator
	{
		using BaseOperator::operator();
		using BaseOperator::dag;

		/** @brief 左操作数寄存器 ID */
		size_t lhs_id;

		/** @brief 右操作数寄存器 ID */
		size_t rhs_id;

		/** @brief 左操作数大小 */
		size_t lhs_size;

		/** @brief 右操作数大小 */
		size_t rhs_size;

		/** @brief 左操作数类型 */
		StateStorageType lhs_type;

		/** @brief 右操作数类型 */
		StateStorageType rhs_type;

		/**
		 * @brief 构造函数（名称版本）
		 * @param reg_lhs 左操作数寄存器名称
		 * @param reg_rhs 右操作数寄存器名称
		 */
		AddAssign_AnyInt_AnyInt(std::string_view reg_lhs, std::string_view reg_rhs)
			: lhs_id(System::get(reg_lhs)), rhs_id(System::get(reg_rhs)),
			lhs_size(System::size_of(lhs_id)), rhs_size(System::size_of(rhs_id)),
			lhs_type(System::type_of(lhs_id)), rhs_type(System::type_of(rhs_id))
		{
			/* Type check */
#ifndef QRAM_Release
			if (lhs_type != UnsignedInteger && lhs_type != SignedInteger)
				throw_invalid_input();
			if (rhs_type != UnsignedInteger && rhs_type != SignedInteger)
				throw_invalid_input();
#endif
		}

		/**
		 * @brief 构造函数（ID 版本）
		 * @param reg_lhs 左操作数寄存器 ID
		 * @param reg_rhs 右操作数寄存器 ID
		 */
		AddAssign_AnyInt_AnyInt(size_t reg_lhs, size_t reg_rhs)
			: lhs_id(reg_lhs), rhs_id(reg_rhs),
			lhs_size(System::size_of(lhs_id)), rhs_size(System::size_of(rhs_id)),
			lhs_type(System::type_of(lhs_id)), rhs_type(System::type_of(rhs_id))
		{
			/* Type check */
#ifndef QRAM_Release
			if (lhs_type != UnsignedInteger && lhs_type != SignedInteger)
				throw_invalid_input();
			if (rhs_type != UnsignedInteger && rhs_type != SignedInteger)
				throw_invalid_input();
#endif
		}

		ClassControllable

		/**
		 * @brief 无符号整数运算实现
		 * @param lhs 左操作数引用
		 * @param l_size 左操作数大小
		 * @param rhs 右操作数值
		 * @param r_size 右操作数大小
		 */
		static void _operate_uint_uint(uint64_t& lhs, size_t l_size, uint64_t rhs, size_t r_size);

		/**
		 * @brief 无符号整数 dagger 运算实现
		 * @param lhs 左操作数引用
		 * @param l_size 左操作数大小
		 * @param rhs 右操作数值
		 * @param r_size 右操作数大小
		 */
		static void _operate_uint_uint_dag(uint64_t& lhs, size_t l_size, uint64_t rhs, size_t r_size);

		/**
		 * @brief 应用累加操作
		 * @param state 系统状态向量
		 */
		void operator()(std::vector<System>& state) const;

		/**
		 * @brief 应用 dagger 操作
		 * @param state 系统状态向量
		 */
		void dag(std::vector<System>& state) const;

#ifdef USE_CUDA
		/**
		 * @brief CUDA 应用累加操作
		 * @param state CUDA 稀疏状态
		 */
		void operator()(CuSparseState& state) const;

		/**
		 * @brief CUDA 应用 dagger 操作
		 * @param state CUDA 稀疏状态
		 */
		void dag(CuSparseState& state) const;
#endif
	};

	/**
	 * @brief 赋值操作（Out-of-place）
	 * @details 实现寄存器复制：register_2 ^= register_1
	 *
	 * 这是量子计算中"复制"操作的标准实现。通过XOR实现，应用两次会恢复原值。
	 *
	 * @note Unitary性质：自伴算子（U^† = U），因为XOR是自逆操作
	 * @note 数据类型：任意类型，只要两个寄存器大小相同
	 * @note 语义：register_2 = register_2 ⊕ register_1
	 *           如果register_2初始为0，则效果为 register_2 = register_1
	 *
	 * @pre register_1和register_2大小必须相同
	 * @pre 所有寄存器必须是激活状态
	 *
	 * @par 示例
	 * @code
	 * auto src = System::add_register("src", UnsignedInteger, 4);
	 * auto dst = System::add_register("dst", UnsignedInteger, 4);
	 * Init_Unsafe(src, 7);  // src = 7
	 * Init_Unsafe(dst, 0);  // dst = 0
	 * // dst = 0 ⊕ 7 = 7
	 * Assign("src", "dst");
	 * // 再次应用：dst = 7 ⊕ 7 = 0 (恢复原值)
	 * Assign("src", "dst");
	 * @endcode
	 */
	struct Assign : SelfAdjointOperator {
		using SelfAdjointOperator::operator();
		using SelfAdjointOperator::dag;

		/** @brief 第一个寄存器 ID */
		size_t register_1;

		/** @brief 第二个寄存器 ID */
		size_t register_2;

		/**
		 * @brief 构造函数（名称版本）
		 * @param reg1 第一个寄存器名称
		 * @param reg2 第二个寄存器名称
		 */
		Assign(std::string_view reg1, std::string_view reg2)
			:register_1(System::get(reg1)), register_2(System::get(reg2))
		{
		}

		/**
		 * @brief 构造函数（ID 版本）
		 * @param reg1 第一个寄存器 ID
		 * @param reg2 第二个寄存器 ID
		 */
		Assign(size_t reg1, size_t reg2)
			:register_1(reg1), register_2(reg2)
		{
		}

		ClassControllable

		/**
		 * @brief 应用赋值操作
		 * @param state 系统状态向量
		 */
		void operator()(std::vector<System>& state) const;

#ifdef USE_CUDA
		/**
		 * @brief CUDA 应用赋值操作
		 * @param state CUDA 稀疏状态
		 */
		void operator()(CuSparseState& state) const;
#endif
	};

	/**
	 * @brief 无符号整数比较操作（Out-of-place）
	 * @details 比较两个无符号整数，输出小于和等于标志
	 *
	 * 实现：
	 * - compare_less_id ^= (left < right)
	 * - compare_equal_id ^= (left == right)
	 *
	 * @note Unitary性质：自伴算子，通过XOR实现
	 * @note 数据类型：left_id和right_id必须是UnsignedInteger，compare_less_id和compare_equal_id必须是Boolean
	 * @note 语义：输出标志是XOR到结果寄存器，如果初始为0则直接存储比较结果
	 *
	 * @pre left_id和right_id必须是UnsignedInteger类型
	 * @pre compare_less_id和compare_equal_id必须是Boolean类型（大小为1）
	 * @pre 所有寄存器必须是激活状态
	 *
	 * @par 示例
	 * @code
	 * auto left = System::add_register("left", UnsignedInteger, 4);
	 * auto right = System::add_register("right", UnsignedInteger, 4);
	 * auto less = System::add_register("less", Boolean, 1);
	 * auto equal = System::add_register("equal", Boolean, 1);
	 * Init_Unsafe(left, 3);
	 * Init_Unsafe(right, 5);
	 * // less = (3 < 5) = 1, equal = (3 == 5) = 0
	 * Compare_UInt_UInt("left", "right", "less", "equal");
	 * @endcode
	 */
	struct Compare_UInt_UInt : SelfAdjointOperator
	{
		using SelfAdjointOperator::operator();
		using SelfAdjointOperator::dag;

		/** @brief 左操作数寄存器 ID */
		size_t left_id;

		/** @brief 右操作数寄存器 ID */
		size_t right_id;

		/** @brief 小于比较结果寄存器 ID */
		size_t compare_less_id;

		/** @brief 等于比较结果寄存器 ID */
		size_t compare_equal_id;

		/**
		 * @brief 构造函数（名称版本）
		 * @param left_register 左操作数寄存器名称
		 * @param right_register 右操作数寄存器名称
		 * @param compare_less 小于结果寄存器名称
		 * @param compare_equal 等于结果寄存器名称
		 */
		Compare_UInt_UInt(
			std::string_view left_register,
			std::string_view right_register,
			std::string_view compare_less,
			std::string_view compare_equal)
			: left_id(System::get(left_register)), right_id(System::get(right_register)),
			compare_less_id(System::get(compare_less)), compare_equal_id(System::get(compare_equal))
		{
			/* Type check */
#ifndef QRAM_Release
			if (System::type_of(left_id) != UnsignedInteger ||
				System::type_of(right_id) != UnsignedInteger ||
				System::type_of(compare_less_id) != Boolean ||
				System::type_of(compare_equal_id) != Boolean)
				throw_invalid_input();
#endif
		}

		/**
		 * @brief 构造函数（ID 版本）
		 * @param lreg 左操作数寄存器 ID
		 * @param rreg 右操作数寄存器 ID
		 * @param compare_less 小于结果寄存器 ID
		 * @param compare_equal 等于结果寄存器 ID
		 */
		Compare_UInt_UInt(size_t lreg, size_t rreg,
			size_t compare_less, size_t compare_equal)
			:left_id(lreg), right_id(rreg),
			compare_less_id(compare_less), compare_equal_id(compare_equal)
		{
			/* Type check */
#ifndef QRAM_Release
			if (System::type_of(left_id) != UnsignedInteger ||
				System::type_of(right_id) != UnsignedInteger ||
				System::type_of(compare_less_id) != Boolean ||
				System::type_of(compare_equal_id) != Boolean)
				throw_invalid_input();
#endif
		}

		ClassControllable

		/**
		 * @brief 应用比较操作
		 * @param state 系统状态向量
		 */
		void operator()(std::vector<System>& state) const;

#ifdef USE_CUDA
		/**
		 * @brief CUDA 应用比较操作
		 * @param state CUDA 稀疏状态
		 */
		void operator()(CuSparseState& state) const;
#endif
	};

	/**
	 * @brief 小于比较操作（Out-of-place）
	 * @details 比较两个无符号整数，仅输出小于标志
	 *
	 * 实现：compare_less_id ^= (left < right)
	 *
	 * @note Unitary性质：自伴算子，通过XOR实现
	 * @note 数据类型：left_id和right_id必须是UnsignedInteger，compare_less_id必须是Boolean
	 * @note 语义：输出标志是XOR到结果寄存器，如果初始为0则直接存储比较结果
	 *
	 * @pre left_id和right_id必须是UnsignedInteger类型
	 * @pre compare_less_id必须是Boolean类型（大小为1）
	 * @pre 所有寄存器必须是激活状态
	 *
	 * @par 示例
	 * @code
	 * auto left = System::add_register("left", UnsignedInteger, 4);
	 * auto right = System::add_register("right", UnsignedInteger, 4);
	 * auto less = System::add_register("less", Boolean, 1);
	 * Init_Unsafe(left, 3);
	 * Init_Unsafe(right, 5);
	 * // less = (3 < 5) = 1
	 * Less_UInt_UInt("left", "right", "less");
	 * @endcode
	 */
	struct Less_UInt_UInt : SelfAdjointOperator
	{
		using SelfAdjointOperator::operator();
		using SelfAdjointOperator::dag;

		/** @brief 左操作数寄存器 ID */
		size_t left_id;

		/** @brief 右操作数寄存器 ID */
		size_t right_id;

		/** @brief 小于结果寄存器 ID */
		size_t compare_less_id;

		/**
		 * @brief 构造函数（名称版本）
		 * @param lreg 左操作数寄存器名称
		 * @param rreg 右操作数寄存器名称
		 * @param compare_less 小于结果寄存器名称
		 */
		Less_UInt_UInt(
			std::string_view lreg,
			std::string_view rreg,
			std::string_view compare_less)
		{
			left_id = System::get(lreg);
			right_id = System::get(rreg);
			compare_less_id = System::get(compare_less);
			/* Type check */
#ifndef QRAM_Release
			if (System::type_of(left_id) != UnsignedInteger ||
				System::type_of(right_id) != UnsignedInteger ||
				System::type_of(compare_less_id) != Boolean)
				throw_invalid_input();
#endif
		}

		/**
		 * @brief 构造函数（ID 版本）
		 * @param lreg 左操作数寄存器 ID
		 * @param rreg 右操作数寄存器 ID
		 * @param compare_less 小于结果寄存器 ID
		 */
		Less_UInt_UInt(size_t lreg, size_t rreg, size_t compare_less)
			: left_id(lreg), right_id(rreg), compare_less_id(compare_less)
		{
			/* Type check */
#ifndef QRAM_Release
			if (System::type_of(left_id) != UnsignedInteger ||
				System::type_of(right_id) != UnsignedInteger ||
				System::type_of(compare_less_id) != Boolean)
				throw_invalid_input();
#endif
		}

		ClassControllable

		/**
		 * @brief 应用小于比较操作
		 * @param state 系统状态向量
		 */
		void operator()(std::vector<System>& state) const;

#ifdef USE_CUDA
		/**
		 * @brief CUDA 应用小于比较操作
		 * @param state CUDA 稀疏状态
		 */
		void operator()(CuSparseState& state) const;
#endif
	};

	/**
	 * @brief 通用交换操作（In-place）
	 * @details 交换两个寄存器的完整值
	 *
	 * 实现：使用std::swap交换两个寄存器的value字段。
	 * 这是一个自伴操作，应用两次等于恒等操作。
	 *
	 * @note Unitary性质：自伴算子，Swap^2 = I
	 * @note 数据类型：任意类型，但两个寄存器大小必须相同
	 * @note 实现细节：直接交换寄存器值，不涉及XOR或算术运算
	 *
	 * @pre id1和id2大小必须相同
	 * @pre 所有寄存器必须是激活状态
	 *
	 * @par 示例
	 * @code
	 * auto reg1 = System::add_register("reg1", UnsignedInteger, 4);
	 * auto reg2 = System::add_register("reg2", UnsignedInteger, 4);
	 * Init_Unsafe(reg1, 5);  // reg1 = 5
	 * Init_Unsafe(reg2, 10); // reg2 = 10
	 * // 交换后: reg1 = 10, reg2 = 5
	 * Swap_General_General("reg1", "reg2");
	 * @endcode
	 */
	struct Swap_General_General : SelfAdjointOperator
	{
		using SelfAdjointOperator::operator();
		using SelfAdjointOperator::dag;

		/** @brief 第一个寄存器 ID */
		size_t id1;

		/** @brief 第二个寄存器 ID */
		size_t id2;

		/**
		 * @brief 构造函数（名称版本）
		 * @param regname1 第一个寄存器名称
		 * @param regname2 第二个寄存器名称
		 * @throws 当寄存器大小不匹配时抛出异常
		 */
		Swap_General_General(std::string_view regname1, std::string_view regname2)
		{
			id1 = System::get(regname1);
			id2 = System::get(regname2);

			/* Type check */
#ifndef QRAM_Release
			if (System::size_of(id1) != System::size_of(id2))
				throw_invalid_input();
#endif		
		}

		/**
		 * @brief 构造函数（ID 版本）
		 * @param regname1 第一个寄存器 ID
		 * @param regname2 第二个寄存器 ID
		 */
		Swap_General_General(size_t regname1, size_t regname2)
			: id1(regname1), id2(regname2)
		{
			/* Type check */
#ifndef QRAM_Release
			if (System::size_of(id1) != System::size_of(id2))
				throw_invalid_input();
#endif		
		}

		ClassControllable

		/**
		 * @brief 应用交换操作
		 * @param state 系统状态向量
		 */
		void operator()(std::vector<System>& state) const;

#ifdef USE_CUDA
		/**
		 * @brief CUDA 应用交换操作
		 * @param state CUDA 稀疏状态
		 */
		void operator()(CuSparseState& state) const;
#endif
	};

	/**
	 * @brief 获取中点操作（Out-of-place）
	 * @details 计算两个无符号整数的中点值：mid ^= (left + right) / 2
	 *
	 * 常用于量子算法中的二分查找和中点计算。
	 *
	 * @note Unitary性质：自伴算子，通过XOR实现
	 * @note 数据类型：left_id、right_id、mid_id都必须是UnsignedInteger
	 * @note 溢出行为：加法可能溢出，但除法后结果正确（整数除法向下取整）
	 *
	 * @pre left_id、right_id、mid_id必须是UnsignedInteger类型
	 * @pre 三个寄存器大小必须相同
	 * @pre 所有寄存器必须是激活状态
	 *
	 * @par 示例
	 * @code
	 * auto left = System::add_register("left", UnsignedInteger, 4);
	 * auto right = System::add_register("right", UnsignedInteger, 4);
	 * auto mid = System::add_register("mid", UnsignedInteger, 4);
	 * Init_Unsafe(left, 0);
	 * Init_Unsafe(right, 10);
	 * // mid = (0 + 10) / 2 = 5
	 * GetMid_UInt_UInt("left", "right", "mid");
	 * @endcode
	 */
	struct GetMid_UInt_UInt : SelfAdjointOperator
	{
		using SelfAdjointOperator::operator();
		using SelfAdjointOperator::dag;

		/** @brief 左操作数寄存器 ID */
		size_t left_id;

		/** @brief 右操作数寄存器 ID */
		size_t right_id;

		/** @brief 中点结果寄存器 ID */
		size_t mid_id;

		/**
		 * @brief 构造函数（名称版本）
		 * @param left_register_ 左操作数寄存器名称
		 * @param right_register_ 右操作数寄存器名称
		 * @param mid_register_ 中点结果寄存器名称
		 */
		GetMid_UInt_UInt(
			std::string_view left_register_,
			std::string_view right_register_,
			std::string_view mid_register_) :
			left_id(System::get(left_register_)),
			right_id(System::get(right_register_)),
			mid_id(System::get(mid_register_))

		{
			/* Type check */
#ifndef QRAM_Release
			if (System::type_of(left_id) != UnsignedInteger ||
				System::type_of(right_id) != UnsignedInteger ||
				System::type_of(mid_id) != UnsignedInteger)
				throw_invalid_input();

			if (System::size_of(left_id) != System::size_of(right_id))
				throw_invalid_input();
			if (System::size_of(right_id) != System::size_of(mid_id))
				throw_invalid_input();
#endif
		}

		/**
		 * @brief 构造函数（ID 版本）
		 * @param left_register_ 左操作数寄存器 ID
		 * @param right_register_ 右操作数寄存器 ID
		 * @param mid_register_ 中点结果寄存器 ID
		 */
		GetMid_UInt_UInt(
			size_t left_register_, size_t right_register_, size_t mid_register_)
			:left_id(left_register_), right_id(right_register_),
			mid_id(mid_register_)
		{
			/* Type check */
#ifndef QRAM_Release
			if (System::type_of(left_id) != UnsignedInteger ||
				System::type_of(right_id) != UnsignedInteger ||
				System::type_of(mid_id) != UnsignedInteger)
				throw_invalid_input();

			if (System::size_of(left_id) != System::size_of(right_id))
				throw_invalid_input();
			if (System::size_of(right_id) != System::size_of(mid_id))
				throw_invalid_input();
#endif
		}

		ClassControllable

		/**
		 * @brief 应用中点计算操作
		 * @param state 系统状态向量
		 */
		void operator()(std::vector<System>& state) const;

#ifdef USE_CUDA
		/**
		 * @brief CUDA 应用中点计算操作
		 * @param state CUDA 稀疏状态
		 */
		void operator()(CuSparseState& state) const;
#endif
	};

	/** @brief 通用算术函数类型 */
	using GenericArithmetic = std::function<std::vector<size_t>(const std::vector<size_t>&)>;

	/**
	 * @brief 自定义算术操作（Out-of-place）
	 * @details 允许用户定义自定义算术函数并应用到量子状态
	 *
	 * 用户可以提供一个函数 func: vector<size_t> -> vector<size_t>，
	 * 操作将输入寄存器的值传入func，然后将输出XOR到输出寄存器。
	 *
	 * @note Unitary性质：自伴算子（U^† = U），因为使用XOR实现
	 * @note 约束：func必须是确定性的纯函数，否则无法保证unitary
	 *
	 * @pre 输入和输出寄存器数量必须在构造函数中正确指定
	 * @pre func必须是确定性的（相同的输入总是产生相同的输出）
	 * @pre 所有寄存器必须是激活状态
	 *
	 * @warning 用户负责确保func不会导致信息丢失（即func应该是输入的确定性函数）
	 *
	 * @par 示例
	 * @code
	 * // 自定义函数：输出 = 输入 * 2
	 * GenericArithmetic double_func = [](const std::vector<size_t>& inputs) {
	 *     return std::vector<size_t>{inputs[0] * 2};
	 * };
	 *
	 * auto inp = System::add_register("inp", UnsignedInteger, 4);
	 * auto out = System::add_register("out", UnsignedInteger, 4);
	 * Init_Unsafe(inp, 7);  // inp = 7
	 *
	 * std::vector<std::string> regs = {"inp", "out"};
	 * CustomArithmetic arith(regs, 1, 1, double_func);
	 * // out = 0 ⊕ (7 * 2) = 14
	 * arith(state);
	 * @endcode
	 */
	struct CustomArithmetic : SelfAdjointOperator
	{
		using SelfAdjointOperator::operator();
		using SelfAdjointOperator::dag;

		/** @brief 输入寄存器 ID 列表 */
		std::vector<size_t> input_ids;

		/** @brief 输出寄存器 ID 列表 */
		std::vector<size_t> output_ids;

		/** @brief 自定义算术函数 */
		GenericArithmetic func;

		/**
		 * @brief 构造函数（ID 版本）
		 * @param input_registers 输入寄存器 ID 列表
		 * @param input_size 输入寄存器数量
		 * @param output_size 输出寄存器数量
		 * @param func 自定义算术函数
		 * @throws 当输入输出大小不匹配时抛出异常
		 */
		CustomArithmetic(const std::vector<size_t>& input_registers,
			size_t input_size, size_t output_size,
			GenericArithmetic func)
			: func(func)
		{
			if (input_ids.size() != output_ids.size())
			{
				throw std::invalid_argument("Input and output registers size does not match.");
			}
			input_ids.resize(input_size);
			output_ids.resize(output_size);
			for (size_t i = 0; i < input_size; ++i)
			{
				input_ids[i] = input_registers[i];
			}
			for (size_t i = 0; i < output_size; ++i)
			{
				output_ids[i] = input_registers[i + input_size];
			}
		}

		/**
		 * @brief 构造函数（名称版本）
		 * @param input_registers 输入寄存器名称列表
		 * @param input_size 输入寄存器数量
		 * @param output_size 输出寄存器数量
		 * @param func 自定义算术函数
		 */
		CustomArithmetic(const std::vector<std::string>& input_registers,
			size_t input_size, size_t output_size, GenericArithmetic func)
			: func(func)
		{
			if ((input_size + output_size) != input_registers.size())
			{
				throw std::invalid_argument("Input registers size does not match input size + output size.");
			}
			input_ids.resize(input_size);
			output_ids.resize(output_size);
			for (size_t i = 0; i < input_size; ++i)
			{
				input_ids[i] = System::get(input_registers[i]);
			}
			for (size_t i = 0; i < output_size; ++i)
			{
				output_ids[i] = System::get(input_registers[i + input_size]);
			}
		}

		ClassControllable

		/**
		 * @brief 应用自定义算术操作
		 * @param state 系统状态向量
		 */
		void operator()(std::vector<System>& state) const
		{
#ifdef SINGLE_THREAD
			for (auto& s : state)
			{
#else
#pragma omp parallel for
			for (int64_t i = 0; i < state.size(); ++i)
			{
				auto& s = state[i];
#endif
				if (ConditionNotSatisfied(s))
					continue;

				/* obtain the value of the input registers */
				std::vector<size_t> input_values(input_ids.size());
				for (size_t i = 0; i < input_ids.size(); ++i)
				{
					input_values[i] = s.GetAs(input_ids[i], uint64_t);
				}
				/* apply the custom function */
				std::vector<size_t> output_values = func(input_values);

				/* write the output reversibly via XOR */
				for (size_t i = 0; i < output_ids.size(); ++i)
				{
					s.get(output_ids[i]).value ^= output_values[i];
				}
			}
		}
	};
} // namespace qram_simulator
