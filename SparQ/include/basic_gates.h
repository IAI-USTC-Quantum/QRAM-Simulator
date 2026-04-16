/**
 * @file basic_gates.h
 * @brief 量子门操作定义
 * @details 包含单量子门、多量子门、受控门等基础量子操作实现，
 *          支持 Phase、Rotation、Pauli (X/Y/Z)、S、T、RX/RY/RZ、SX、U2、U3 等标准门
 */

#pragma once
#include "sparse_state_simulator.h"
#include "matrix.h"
#include <Eigen/Eigen>

namespace qram_simulator
{
	/** @namespace qram_simulator
	 * @brief QRAM 稀疏态模拟器命名空间
	 */

	/**
	 * @brief 量子门基类
	 * @details 所有量子门的基类，管理寄存器 ID 和量子位索引
	 */
	struct GateBase {
		/** @brief 寄存器 ID */
		size_t id;

		/** @brief 量子位索引 */
		size_t digit;

		/**
		 * @brief 构造函数（名称 + 位索引）
		 * @param reg_ 寄存器名称
		 * @param digit_ 量子位索引
		 * @throws 当位索引超出范围时抛出异常
		 */
		GateBase(std::string_view reg_, size_t digit_) 
			: id(System::get(reg_)), digit(digit_) 
		{
			size_t size = System::size_of(id);
			if (digit >= size)
				throw_invalid_input();
		}

		/**
		 * @brief 构造函数（ID + 位索引）
		 * @param id_ 寄存器 ID
		 * @param digit_ 量子位索引
		 */
		GateBase(size_t id_, size_t digit_) : id(id_), digit(digit_)
		{
			size_t size = System::size_of(id);
			if (digit >= size)
				throw_invalid_input();
		}

		/**
		 * @brief 构造函数（仅名称，默认位索引为0）
		 * @param reg_ 寄存器名称
		 */
		GateBase(std::string_view reg_) : GateBase(System::get(reg_), 0) {}

		/**
		 * @brief 构造函数（仅ID，默认位索引为0）
		 * @param id_ 寄存器 ID
		 */
		GateBase(size_t id_) : GateBase(id_, 0) {}
	};

	/**
	 * @brief 相位门
	 * @details 在指定量子位上应用相位旋转 e^{iλ}
	 */
	struct Phase_Bool : BaseOperator, GateBase {
		using BaseOperator::operator();
		using BaseOperator::dag;

		/** @brief 相位角（弧度） */
		double lambda;

		ClassControllable

		/**
		 * @brief 构造函数（名称 + 位索引 + 相位角）
		 * @param reg_ 寄存器名称
		 * @param digit_ 量子位索引
		 * @param lambda_ 相位角（弧度）
		 */
		Phase_Bool(std::string_view reg_, size_t digit_, double lambda_)
			: GateBase(System::get(reg_), digit_), lambda(lambda_)
		{
		}

		/**
		 * @brief 构造函数（ID + 位索引 + 相位角）
		 * @param id_ 寄存器 ID
		 * @param digit_ 量子位索引
		 * @param lambda_ 相位角（弧度）
		 */
		Phase_Bool(size_t id_, size_t digit_, double lambda_)
			: GateBase(id_, digit_), lambda(lambda_)
		{
		}

		/**
		 * @brief 构造函数（名称 + 相位角，默认位索引为0）
		 * @param reg_ 寄存器名称
		 * @param lambda_ 相位角（弧度）
		 */
		Phase_Bool(std::string_view reg_, double lambda_) : Phase_Bool(reg_, 0, lambda_) {}

		/**
		 * @brief 构造函数（ID + 相位角，默认位索引为0）
		 * @param id_ 寄存器 ID
		 * @param lambda_ 相位角（弧度）
		 */
		Phase_Bool(size_t id_, double lambda_) : Phase_Bool(id_, 0, lambda_) {}

		/**
		 * @brief 应用相位门操作
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
	 * @brief 旋转门基类
	 * @details 实现通用的 2x2 酉矩阵旋转操作
	 */
	struct Rot_Bool : BaseOperator, GateBase {
		using BaseOperator::operator();
		using BaseOperator::dag;

		/** @brief 角度函数类型 */
		using angle_function_t = std::function<u22_t(size_t)>;

		/** @brief 位掩码 */
		uint64_t mask;

		/** @brief 2x2 旋转矩阵 */
		u22_t mat;

		ClassControllable

		/**
		 * @brief 构造函数（名称 + 位索引 + 矩阵）
		 * @param reg_ 寄存器名称
		 * @param digit_ 量子位索引
		 * @param mat 2x2 酉矩阵
		 */
		Rot_Bool(std::string_view reg_, size_t digit_, u22_t mat)
			: GateBase(System::get(reg_), digit_), mat(mat)
		{
			mask = pow2(digit);
		}

		/**
		 * @brief 构造函数（ID + 位索引 + 矩阵）
		 * @param id_ 寄存器 ID
		 * @param digit_ 量子位索引
		 * @param mat 2x2 酉矩阵
		 */
		Rot_Bool(size_t id_, size_t digit_, u22_t mat)
			: GateBase(id_, digit_), mat(mat)
		{
			mask = pow2(digit);
		}

		/**
		 * @brief 构造函数（名称 + 矩阵，单比特寄存器）
		 * @param reg_ 寄存器名称
		 * @param mat 2x2 酉矩阵
		 * @throws 当寄存器大小不为1时抛出异常
		 */
		Rot_Bool(std::string_view reg_, u22_t mat) : Rot_Bool(reg_, 0, mat) {
			if (System::size_of(reg_) != 1) {
				throw_invalid_input();
			}
		}

		/**
		 * @brief 构造函数（ID + 矩阵，单比特寄存器）
		 * @param id_ 寄存器 ID
		 * @param mat 2x2 酉矩阵
		 * @throws 当寄存器大小不为1时抛出异常
		 */
		Rot_Bool(size_t id_, u22_t mat) : Rot_Bool(id_, 0, mat) {
			if (System::size_of(id_) != 1) {
				throw_invalid_input();
			}
		}

		/**
		 * @brief 在指定范围执行旋转操作
		 * @param l 左边界
		 * @param r 右边界
		 * @param state 系统状态向量
		 */
		void operate(size_t l, size_t r, std::vector<System>& state) const;

		/**
		 * @brief 检查矩阵是否为对角矩阵
		 * @param data 2x2 矩阵
		 * @return 是否为对角矩阵
		 */
		static bool _is_diagonal(const u22_t& data);

		/**
		 * @brief 对角矩阵操作实现
		 * @param l 左边界
		 * @param r 右边界
		 * @param state 系统状态向量
		 * @param mat 2x2 对角矩阵
		 */
		void _operate_diagonal(size_t l, size_t r,
			std::vector<System>& state, const u22_t& mat) const;

		/**
		 * @brief 检查矩阵是否为反对角矩阵
		 * @param data 2x2 矩阵
		 * @return 是否为反对角矩阵
		 */
		static bool _is_off_diagonal(const u22_t& data);

		/**
		 * @brief 反对角矩阵操作实现
		 * @param l 左边界
		 * @param r 右边界
		 * @param state 系统状态向量
		 * @param mat 2x2 反对角矩阵
		 */
		void _operate_off_diagonal(size_t l, size_t r,
			std::vector<System>& state, const u22_t& mat) const;

		/**
		 * @brief 一般矩阵操作实现
		 * @param l 左边界
		 * @param r 右边界
		 * @param state 系统状态向量
		 * @param mat 2x2 一般酉矩阵
		 */
		void _operate_general(size_t l, size_t r,
			std::vector<System>& state, const u22_t& mat) const;

		/**
		 * @brief 成对操作（|0> 和 |1> 都存在的分支）
		 * @param zero |0> 分支索引
		 * @param one |1> 分支索引
		 * @param state 系统状态向量
		 */
		void operate_pair(size_t zero, size_t one, std::vector<System>& state) const;

		/**
		 * @brief 单独操作 |0> 分支
		 * @param zero |0> 分支索引
		 * @param state 系统状态向量
		 */
		void operate_alone_zero(size_t zero, std::vector<System>& state) const;

		/**
		 * @brief 单独操作 |1> 分支
		 * @param one |1> 分支索引
		 * @param state 系统状态向量
		 */
		void operate_alone_one(size_t one, std::vector<System>& state) const;

		/**
		 * @brief 应用旋转门操作
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
		 * @brief CUDA 对角矩阵操作
		 */
		void cu_operate_diagonal(CuSparseState& s, complex_t u00, complex_t u11) const;

		/**
		 * @brief CUDA 反对角矩阵操作
		 */
		void cu_operate_off_diagonal(CuSparseState& s, complex_t u01, complex_t u10) const;

		/**
		 * @brief CUDA 一般矩阵操作
		 */
		void cu_operate_general(CuSparseState& s, complex_t u00, complex_t u01, complex_t u10, complex_t u11) const;

		/**
		 * @brief CUDA 应用旋转门操作
		 * @param s CUDA 稀疏状态
		 */
		void operator()(CuSparseState& s) const;
#endif
	};

	/**
	 * @brief X 门（Pauli-X / NOT 门）
	 * @details 翻转量子位状态 |0> <-> |1>
	 */
	struct Xgate_Bool : SelfAdjointOperator, GateBase {
		using SelfAdjointOperator::operator();
		using SelfAdjointOperator::dag;

		ClassControllable

		/**
		 * @brief 构造函数（名称 + 位索引）
		 * @param reg_ 寄存器名称
		 * @param digit_ 量子位索引
		 */
		Xgate_Bool(std::string_view reg_, size_t digit_)
			: GateBase(System::get(reg_), digit_)
		{
		}

		/**
		 * @brief 构造函数（ID + 位索引）
		 * @param id_ 寄存器 ID
		 * @param digit_ 量子位索引
		 */
		Xgate_Bool(size_t id_, size_t digit_)
			: GateBase(id_, digit_)
		{
		}

		/**
		 * @brief 构造函数（名称，默认位索引为0）
		 * @param reg_ 寄存器名称
		 */
		Xgate_Bool(std::string_view reg_) : Xgate_Bool(reg_, 0) {}

		/**
		 * @brief 构造函数（ID，默认位索引为0）
		 * @param id_ 寄存器 ID
		 */
		Xgate_Bool(size_t id_) : Xgate_Bool(id_, 0) {}

		/**
		 * @brief 应用 X 门操作
		 * @param state 系统状态向量
		 */
		void operator()(std::vector<System>& state) const;

#ifdef USE_CUDA
		/**
		 * @brief CUDA 应用 X 门操作
		 * @param state CUDA 稀疏状态
		 */
		void operator()(CuSparseState& state) const;
#endif
	};

	/**
	 * @brief Y 门（Pauli-Y 门）
	 * @details 在 Bloch 球上绕 Y 轴旋转 π 角度
	 */
	struct Ygate_Bool : SelfAdjointOperator, GateBase {
		using SelfAdjointOperator::operator();
		using SelfAdjointOperator::dag;
		using GateBase::GateBase;

		ClassControllable

		void operator()(std::vector<System>& state) const;

		/**
		 * @brief 提取 Y 门的矩阵表示
		 * @return Y 门的稠密矩阵
		 */
		DenseMatrix<complex_t> extract_matrix();
	};

	/**
	 * @brief Z 门（Pauli-Z 门）
	 * @details 相位翻转门，施加 π 相位
	 */
	struct Zgate_Bool : Phase_Bool
	{
		using Phase_Bool::operator();
		using Phase_Bool::dag;
		using Phase_Bool::Phase_Bool;

// Convenience constructors for single-qubit Z gate (lambda = pi)
		Zgate_Bool(std::string_view reg_) : Phase_Bool(reg_, 0, pi) {}

		/**
		 * @brief 构造函数（ID，默认位索引为0）
		 * @param id_ 寄存器 ID
		 */
		Zgate_Bool(size_t id_) : Phase_Bool(id_, 0, pi) {}
//void display() const override;
	};

	/**
	 * @brief S 门
	 * @details 相位门，施加 π/2 相位
	 */
	struct Sgate_Bool : Phase_Bool
	{
		using Phase_Bool::operator();
		using Phase_Bool::dag;
		using Phase_Bool::Phase_Bool;

// Convenience constructors for S gate (lambda = pi/2)
		Sgate_Bool(std::string_view reg_) : Phase_Bool(reg_, 0, pi / 2) {}

		/**
		 * @brief 构造函数（ID，默认位索引为0）
		 * @param id_ 寄存器 ID
		 */
		Sgate_Bool(size_t id_) : Phase_Bool(id_, 0, pi / 2) {}
//void display() const override;
	};

	/**
	 * @brief T 门
	 * @details 相位门，施加 π/4 相位
	 */
	struct Tgate_Bool : Phase_Bool
	{
		using Phase_Bool::operator();
		using Phase_Bool::dag;
		using Phase_Bool::Phase_Bool;

// Convenience constructors for T gate (lambda = pi/4)
		Tgate_Bool(std::string_view reg_) : Phase_Bool(reg_, 0, pi / 4) {}

		/**
		 * @brief 构造函数（ID，默认位索引为0）
		 * @param id_ 寄存器 ID
		 */
		Tgate_Bool(size_t id_) : Phase_Bool(id_, 0, pi / 4) {}
//void display() const override;
	};

	/**
	 * @brief RX 门（绕 X 轴旋转）
	 * @details 在 Bloch 球上绕 X 轴旋转指定角度
	 */
	struct RXgate_Bool : Rot_Bool
	{
		using Rot_Bool::operator();
		using Rot_Bool::dag;

		/** @brief 旋转矩阵 */
		u22_t mat;

		/**
		 * @brief 构造函数（名称 + 位索引 + 角度）
		 * @param reg_ 寄存器名称
		 * @param digit_ 量子位索引
		 * @param angle_ 旋转角度
		 */
		RXgate_Bool(std::string_view reg_, size_t digit_, double angle_);

		/**
		 * @brief 构造函数（ID + 位索引 + 角度）
		 * @param id_ 寄存器 ID
		 * @param digit_ 量子位索引
		 * @param angle_ 旋转角度
		 */
		RXgate_Bool(size_t id_, size_t digit_, double angle_);

		/**
		 * @brief 构造函数（名称 + 角度，默认位索引为0）
		 * @param reg_ 寄存器名称
		 * @param angle_ 旋转角度
		 */
		RXgate_Bool(std::string_view reg_, double angle_) : RXgate_Bool(reg_, 0, angle_) {}

		/**
		 * @brief 构造函数（ID + 角度，默认位索引为0）
		 * @param id_ 寄存器 ID
		 * @param angle_ 旋转角度
		 */
		RXgate_Bool(size_t id_, double angle_) : RXgate_Bool(id_, 0, angle_) {}

		/**
		 * @brief 应用 RX 门操作
		 * @param state 系统状态向量
		 */
		inline void operator()(std::vector<System>& state) const
		{
			Rot_Bool::operator()(state);
		}
	};

	/**
	 * @brief RY 门（绕 Y 轴旋转）
	 * @details 在 Bloch 球上绕 Y 轴旋转指定角度
	 */
	struct RYgate_Bool : Rot_Bool
	{
		using Rot_Bool::operator();
		using Rot_Bool::dag;

		/** @brief 旋转矩阵 */
		u22_t mat;

		/**
		 * @brief 构造函数（名称 + 位索引 + 角度）
		 * @param reg 寄存器名称
		 * @param digit_ 量子位索引
		 * @param angle_ 旋转角度
		 */
		RYgate_Bool(std::string_view reg, size_t digit_, double angle_);

		/**
		 * @brief 构造函数（ID + 位索引 + 角度）
		 * @param id_ 寄存器 ID
		 * @param digit_ 量子位索引
		 * @param angle_ 旋转角度
		 */
		RYgate_Bool(size_t id_, size_t digit_, double angle_);

		/**
		 * @brief 构造函数（名称 + 角度，默认位索引为0）
		 * @param reg_ 寄存器名称
		 * @param angle_ 旋转角度
		 */
		RYgate_Bool(std::string_view reg_, double angle_) : RYgate_Bool(reg_, 0, angle_) {}

		/**
		 * @brief 构造函数（ID + 角度，默认位索引为0）
		 * @param id_ 寄存器 ID
		 * @param angle_ 旋转角度
		 */
		RYgate_Bool(size_t id_, double angle_) : RYgate_Bool(id_, 0, angle_) {}

		/**
		 * @brief 应用 RY 门操作
		 * @param state 系统状态向量
		 */
		inline void operator()(std::vector<System>& state) const
		{
			Rot_Bool::operator()(state);
		}
	};

	/**
	 * @brief RZ 门（绕 Z 轴旋转）
	 * @details 在 Bloch 球上绕 Z 轴旋转指定角度
	 */
	struct RZgate_Bool : BaseOperator, GateBase {
		using BaseOperator::operator();
		using BaseOperator::dag;

		/** @brief 旋转角度 */
		double angle;

		ClassControllable

		/**
		 * @brief 构造函数（名称 + 位索引 + 角度）
		 * @param reg_ 寄存器名称
		 * @param digit_ 量子位索引
		 * @param angle_ 旋转角度
		 */
		RZgate_Bool(std::string_view reg_, size_t digit_, double angle_);

		/**
		 * @brief 构造函数（ID + 位索引 + 角度）
		 * @param id_ 寄存器 ID
		 * @param digit_ 量子位索引
		 * @param angle_ 旋转角度
		 */
		RZgate_Bool(size_t id_, size_t digit_, double angle_);

		/**
		 * @brief 构造函数（名称 + 角度，默认位索引为0）
		 * @param reg_ 寄存器名称
		 * @param angle_ 旋转角度
		 */
		RZgate_Bool(std::string_view reg_, double angle_) : RZgate_Bool(reg_, 0, angle_) {}

		/**
		 * @brief 构造函数（ID + 角度，默认位索引为0）
		 * @param id_ 寄存器 ID
		 * @param angle_ 旋转角度
		 */
		RZgate_Bool(size_t id_, double angle_) : RZgate_Bool(id_, 0, angle_) {}

		/**
		 * @brief 应用 RZ 门操作
		 * @param state 系统状态向量
		 */
		void operator()(std::vector<System>& state) const;
	};

	/**
	 * @brief SX 门（X 的平方根门）
	 * @details sqrt(X) 门，X 门的一半旋转
	 */
	struct SXgate_Bool : Rot_Bool
	{
		using Rot_Bool::operator();
		using Rot_Bool::dag;

		/** @brief 旋转矩阵 */
		u22_t mat;

		/**
		 * @brief 构造函数（名称 + 位索引）
		 * @param reg_ 寄存器名称
		 * @param digit_ 量子位索引
		 */
		SXgate_Bool(std::string_view reg_, size_t digit_);

		/**
		 * @brief 构造函数（ID + 位索引）
		 * @param id_ 寄存器 ID
		 * @param digit_ 量子位索引
		 */
		SXgate_Bool(size_t id_, size_t digit_);

		/**
		 * @brief 构造函数（名称，默认位索引为0）
		 * @param reg_ 寄存器名称
		 */
		SXgate_Bool(std::string_view reg_) : SXgate_Bool(reg_, 0) {}

		/**
		 * @brief 构造函数（ID，默认位索引为0）
		 * @param id_ 寄存器 ID
		 */
		SXgate_Bool(size_t id_) : SXgate_Bool(id_, 0) {}

		/**
		 * @brief 应用 SX 门操作
		 * @param state 系统状态向量
		 */
		inline void operator()(std::vector<System>& state) const
		{
			Rot_Bool::operator()(state);
		}
	};

	/**
	 * @brief U2 门（通用单量子门，2参数）
	 * @details 通用单量子门，使用 phi 和 lambda 两个参数
	 */
	struct U2gate_Bool : Rot_Bool
	{
		using Rot_Bool::operator();
		using Rot_Bool::dag;

		/** @brief 旋转矩阵 */
		u22_t mat;

		/** @brief phi 参数 */
		double phi;

		/** @brief lambda 参数 */
		double lambda;

		/**
		 * @brief 构造函数（名称 + 位索引 + phi + lambda）
		 * @param reg_ 寄存器名称
		 * @param digit_ 量子位索引
		 * @param phi phi 参数
		 * @param lambda lambda 参数
		 */
		U2gate_Bool(std::string_view reg_, size_t digit_, double phi, double lambda);

		/**
		 * @brief 构造函数（ID + 位索引 + phi + lambda）
		 * @param id_ 寄存器 ID
		 * @param digit_ 量子位索引
		 * @param phi phi 参数
		 * @param lambda lambda 参数
		 */
		U2gate_Bool(size_t id_, size_t digit_, double phi, double lambda);

		/**
		 * @brief 构造函数（名称 + phi + lambda，默认位索引为0）
		 * @param reg_ 寄存器名称
		 * @param phi phi 参数
		 * @param lambda lambda 参数
		 */
		U2gate_Bool(std::string_view reg_, double phi, double lambda) : U2gate_Bool(reg_, 0, phi, lambda) {}

		/**
		 * @brief 构造函数（ID + phi + lambda，默认位索引为0）
		 * @param id_ 寄存器 ID
		 * @param phi phi 参数
		 * @param lambda lambda 参数
		 */
		U2gate_Bool(size_t id_, double phi, double lambda) : U2gate_Bool(id_, 0, phi, lambda) {}

		/**
		 * @brief 应用 U2 门操作
		 * @param state 系统状态向量
		 */
		inline void operator()(std::vector<System>& state) const
		{
			Rot_Bool::operator()(state);
		}
	};

	/**
	 * @brief U3 门（通用单量子门，3参数）
	 * @details 最通用的单量子门，使用 theta、phi 和 lambda 三个参数
	 */
	struct U3gate_Bool : Rot_Bool
	{
		using Rot_Bool::operator();
		using Rot_Bool::dag;

		/** @brief theta 参数 */
		double theta;

		/** @brief phi 参数 */
		double phi;

		/** @brief lambda 参数 */
		double lambda;

		/**
		 * @brief 构造函数（名称 + 位索引 + theta + phi + lambda）
		 * @param reg 寄存器名称
		 * @param digit_ 量子位索引
		 * @param theta theta 参数
		 * @param phi phi 参数
		 * @param lambda lambda 参数
		 */
		U3gate_Bool(std::string_view reg, size_t digit_, double theta, double phi, double lambda);

		/**
		 * @brief 构造函数（ID + 位索引 + theta + phi + lambda）
		 * @param id_ 寄存器 ID
		 * @param digit_ 量子位索引
		 * @param theta theta 参数
		 * @param phi phi 参数
		 * @param lambda lambda 参数
		 */
		U3gate_Bool(size_t id_, size_t digit_, double theta, double phi, double lambda);

		/**
		 * @brief 构造函数（名称 + theta + phi + lambda，默认位索引为0）
		 * @param reg_ 寄存器名称
		 * @param theta theta 参数
		 * @param phi phi 参数
		 * @param lambda lambda 参数
		 */
		U3gate_Bool(std::string_view reg_, double theta, double phi, double lambda) : U3gate_Bool(reg_, 0, theta, phi, lambda) {}

		/**
		 * @brief 构造函数（ID + theta + phi + lambda，默认位索引为0）
		 * @param id_ 寄存器 ID
		 * @param theta theta 参数
		 * @param phi phi 参数
		 * @param lambda lambda 参数
		 */
		U3gate_Bool(size_t id_, double theta, double phi, double lambda) : U3gate_Bool(id_, 0, theta, phi, lambda) {}

		/**
		 * @brief 应用 U3 门操作
		 * @param state 系统状态向量
		 */
		inline void operator()(std::vector<System>& state) const
		{
			Rot_Bool::operator()(state);
		}
	};
}
