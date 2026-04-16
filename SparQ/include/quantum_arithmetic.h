/**
 * @file quantum_arithmetic.h
 * @brief 量子算术运算定义
 * @details 实现各种量子算术操作，包括翻转、移位、乘法、加法、比较等运算
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
	 * @details 翻转通用寄存器中的所有位
	 * @note 数据类型：通用目的，剩余高位也会被翻转
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
	 * @details 交换两个寄存器中指定位置的比特
	 * @note 数据类型：通用目的，输入位可能溢出
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
		 */
		Swap_Bool_Bool(std::string_view reg1, size_t d1,
			std::string_view reg2, size_t d2)
			: lhs(System::get(reg1)), rhs(System::get(reg2)),
			digit1(d1), digit2(d2)
		{
		}

		/**
		 * @brief 构造函数（ID 版本）
		 * @param id1 第一个寄存器 ID
		 * @param d1 第一个位索引
		 * @param id2 第二个寄存器 ID
		 * @param d2 第二个位索引
		 */
		Swap_Bool_Bool(size_t id1, size_t d1,
			size_t id2, size_t d2) : lhs(id1), rhs(id2), digit1(d1), digit2(d2)
		{
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
	 * @brief 左移操作
	 * @details 将寄存器值左移指定位数，最高位循环到最低位
	 * @note 数据类型：建议使用无符号整数
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
		 */
		ShiftLeft(std::string_view reg1, size_t d) 
			:register_1(System::get(reg1)), digit(d)
		{
		}

		/**
		 * @brief 构造函数（ID 版本）
		 * @param reg1 寄存器 ID
		 * @param d 移位位数
		 */
		ShiftLeft(size_t reg1, size_t d)
			:register_1(reg1), digit(d)
		{
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
	 * @brief 右移操作
	 * @details 将寄存器值右移指定位数，最低位循环到最高位
	 * @note 数据类型：建议使用无符号整数
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
		 */
		ShiftRight(std::string_view reg1, size_t d)
			:register_1(System::get(reg1)), digit(d)
		{
		}

		/**
		 * @brief 构造函数（ID 版本）
		 * @param reg1 寄存器 ID
		 * @param d 移位位数
		 */
		ShiftRight(size_t reg1, size_t d)
			:register_1(reg1), digit(d)
		{
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
	 * @brief 无符号整数乘常量操作
	 * @details z = x * mult，x 为寄存器值，mult 为常量
	 * @note 数据类型：无符号整数，结果可能溢出
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
	 * @brief 累加乘常量操作
	 * @details z += x * mult
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
		 * @param reg_out 输出寄存器名称
		 */
		Add_Mult_UInt_ConstUInt(std::string_view reg_in, size_t mult, std::string_view reg_out)
			: mult_int(mult), lhs(System::get(reg_in)), res(System::get(reg_out))
		{
		}

		/**
		 * @brief 构造函数（ID 版本）
		 * @param reg_in 输入寄存器 ID
		 * @param mult 乘数常量
		 * @param reg_out 输出寄存器 ID
		 */
		Add_Mult_UInt_ConstUInt(size_t reg_in, size_t mult, size_t reg_out)
			: mult_int(mult), lhs(reg_in), res(reg_out)
		{
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
	 * @brief 无符号整数加法操作
	 * @details z = x + y
	 * @note 数据类型：无符号整数，结果可能溢出
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
	 * @brief 原地无符号整数加法操作
	 * @details y += x，结果存储在 rhs 中
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
		 * @param lhs_ 左操作数寄存器名称
		 * @param rhs_ 右操作数寄存器名称
		 */
		Add_UInt_UInt_InPlace(std::string_view lhs_, std::string_view rhs_)
			:lhs(System::get(lhs_)), rhs(System::get(rhs_))
		{
		}

		/**
		 * @brief 构造函数（ID 版本）
		 * @param lhs_ 左操作数寄存器 ID
		 * @param rhs_ 右操作数寄存器 ID
		 */
		Add_UInt_UInt_InPlace(size_t lhs_, size_t rhs_)
			: lhs(lhs_), rhs(rhs_)
		{
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
	 * @brief 无符号整数加常量操作
	 * @details z = x + add，add 为常量
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
	 * @brief 加常量操作（原地）
	 * @details reg_in += add
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
		 * @param reg_in_ 输入寄存器名称
		 * @param add 加数常量
		 */
		Add_ConstUInt(std::string_view reg_in_, size_t add) :
			reg_in(System::get(reg_in_)), add_int(add)
		{
		}

		/**
		 * @brief 构造函数（ID 版本）
		 * @param reg_in 输入寄存器 ID
		 * @param add 加数常量
		 */
		Add_ConstUInt(size_t reg_in, size_t add)
			: reg_in(reg_in), add_int(add)
		{
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
	 * @brief 除法平方根反余弦操作
	 * @details z = arccos(sqrt(y / x))
	 * @note 数据类型：无符号整数输入，有理数输出
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
		if (System::type_of(register_lhs) != UnsignedInteger ||
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
		Div_Sqrt_Arccos_Int_Int(size_t reg_lhs, size_t reg_rhs, size_t reg_out)
			:register_lhs(reg_lhs),
			register_rhs(reg_rhs),
			register_out(reg_out)
		{
			/* Type check */
#ifndef QRAM_Release
			if (System::type_of(register_lhs) != UnsignedInteger ||
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
	 * @brief 平方根除法反余弦操作
	 * @details z = arccos(sqrt(y) / x)
	 * @note 数据类型：有符号/无符号整数输入，有理数输出
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
	 * @brief 获取旋转角度操作
	 * @details 计算两个整数之间的旋转角度
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
	 * @brief 任意整数累加操作
	 * @details y += x，支持有符号和无符号整数
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
	 * @brief 赋值操作
	 * @details y = x，通过 XOR 实现
	 * @note 数据类型：通用
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
	 * @brief 无符号整数比较操作
	 * @details |l>|r>|0>|0> -> |l>|r>|l < r?>|l == r?>
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
	 * @brief 小于比较操作
	 * @details |l>|r>|0> -> |l>|r>|l < r?>
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
	 * @brief 通用交换操作
	 * @details 交换两个寄存器的值
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
	 * @brief 获取中点操作
	 * @details |l>|r>|0> -> |l>|r>|mid>
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
	 * @brief 自定义算术操作
	 * @details 允许用户定义自定义算术函数并应用到量子状态
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
