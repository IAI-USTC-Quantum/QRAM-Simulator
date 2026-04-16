/**
 * @file rot.h
 * @brief 旋转门和态制备操作定义
 * @details 实现通用酉旋转门和量子态制备操作，
 *          支持任意维度的酉矩阵旋转和 Schmidt 分解态制备
 */

#pragma once
#include "quantum_interfere_basic.h"
#include "matrix.h"

namespace qram_simulator
{
	/** @namespace qram_simulator
	 * @brief QRAM 稀疏态模拟器命名空间
	 */

	/**
	 * @brief 通用酉旋转门
	 * @details 对整数寄存器应用任意维度的酉矩阵
	 */
	struct Rot_GeneralUnitary : BaseOperator
	{
		using BaseOperator::operator();
		using BaseOperator::dag;

		/** @brief 酉矩阵类型 */
		using unitary_t = DenseMatrix<complex_t>;

		/** @brief 酉矩阵 */
		unitary_t mat;

		/** @brief 寄存器 ID */
		size_t id;

		/** @brief 量子位数 */
		size_t n_digits;

		/** @brief 完整状态大小 */
		size_t full_size;

		ClassControllable

		/**
		 * @brief 构造函数（名称版本）
		 * @param reg_in 寄存器名称
		 * @param mat_ 酉矩阵
		 * @throws 当矩阵大小与寄存器大小不匹配时抛出异常
		 */
		Rot_GeneralUnitary(std::string_view reg_in, const unitary_t &mat_)
			: id(System::get(reg_in)), mat(mat_)
		{
			n_digits = System::size_of(id);
			full_size = pow2(n_digits);

			if (full_size != mat_.size)
				throw_invalid_input("Matrix size does not match the register's size.");
		}

		/**
		 * @brief 构造函数（ID 版本）
		 * @param reg_in 寄存器 ID
		 * @param mat_ 酉矩阵
		 * @throws 当矩阵大小与寄存器大小不匹配时抛出异常
		 */
		Rot_GeneralUnitary(size_t reg_in, const unitary_t &mat_)
			: id(reg_in), mat(mat_)
		{
			n_digits = System::size_of(id);
			full_size = pow2(n_digits);

			if (full_size != mat_.size)
				throw_invalid_input("Matrix size does not match the register's size.");
		}

		/**
		 * @brief 原地桶操作
		 * @param positions 位置列表
		 * @param state 系统状态向量
		 * @param dagger 是否为 dagger 操作
		 */
		void operate_bucket_inplace(const std::vector<size_t>& positions, std::vector<System>& state, bool dagger) const;

		/**
		 * @brief 执行旋转操作
		 * @param state 系统状态向量
		 * @param dagger 是否为 dagger 操作
		 */
		void operate(std::vector<System>& state, bool dagger) const;

		/**
		 * @brief 应用旋转操作
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
		 * @brief CUDA 应用旋转操作
		 * @param state CUDA 稀疏状态
		 */
		virtual void operator()(CuSparseState& state) const;

		/**
		 * @brief CUDA 应用 dagger 操作
		 * @param state CUDA 稀疏状态
		 */
		virtual void dag(CuSparseState& state) const;
#endif
	};

	/**
	 * @brief 使用 Schmidt 分解构建态制备酉矩阵
	 * @param vec 目标态向量
	 * @return 态制备酉矩阵
	 */
	DenseMatrix<complex_t> stateprep_unitary_build_schmidt(const std::vector<complex_t>& vec);

	/**
	 * @brief 通用态制备操作
	 * @details 使用酉矩阵将 |0...0> 制备为目标量子态
	 */
	struct Rot_GeneralStatePrep : BaseOperator {
		using BaseOperator::operator();
		using BaseOperator::dag;

		/** @brief 酉矩阵类型 */
		using unitary_t = DenseMatrix<complex_t>;

		/** @brief 目标态向量 */
		std::vector<complex_t> vec;

		/** @brief 寄存器 ID */
		size_t id;

		/** @brief 量子位数 */
		size_t n_digits;

		/** @brief 完整状态大小 */
		size_t full_size;

		/** @brief 通用旋转门 */
		Rot_GeneralUnitary rot_general;

		ClassControllable

		/**
		 * @brief 构造函数（名称版本）
		 * @param reg_in 寄存器名称
		 * @param vec_ 目标态向量
		 */
		Rot_GeneralStatePrep(std::string_view reg_in, const std::vector<complex_t> &vec);

		/**
		 * @brief 构造函数（ID 版本）
		 * @param reg_in 寄存器 ID
		 * @param vec_ 目标态向量
		 */
		Rot_GeneralStatePrep(size_t reg_in, const std::vector<complex_t> &vec);

		/**
		 * @brief 应用态制备操作
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
		 * @brief CUDA 应用态制备操作
		 * @param state CUDA 稀疏状态
		 */
		virtual void operator()(CuSparseState& state) const;

		/**
		 * @brief CUDA 应用 dagger 操作
		 * @param state CUDA 稀疏状态
		 */
		virtual void dag(CuSparseState& state) const;
#endif
	};
}
