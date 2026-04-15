#pragma once

// Windows MSVC requires _USE_MATH_DEFINES for M_PI
#ifdef _WIN32
#define _USE_MATH_DEFINES
#endif

#include "sparse_state_simulator.h"
#include "matrix.h"
#include <Eigen/Eigen>

namespace qram_simulator
{
	/* Base class for all gates. */
	struct GateBase {

		size_t id;
		size_t digit;
		GateBase(std::string_view reg_, size_t digit_) 
			: id(System::get(reg_)), digit(digit_) 
		{
			size_t size = System::size_of(id);
			if (digit >= size)
				throw_invalid_input();
		}
		GateBase(size_t id_, size_t digit_) : id(id_), digit(digit_)
		{
			size_t size = System::size_of(id);
			if (digit >= size)
				throw_invalid_input();
		}
		GateBase(std::string_view reg_) : GateBase(System::get(reg_), 0) {}
		GateBase(size_t id_) : GateBase(id_, 0) {}
		//virtual void display() const { fmt::print("Base Gate display function.\n"); };
	};

	struct Phase_Bool : BaseOperator, GateBase {
		using BaseOperator::operator();
		using BaseOperator::dag;

		double lambda;
		ClassControllable
		Phase_Bool(std::string_view reg_, size_t digit_, double lambda_)
			: GateBase(System::get(reg_), digit_), lambda(lambda_)
		{
		}
		Phase_Bool(size_t id_, size_t digit_, double lambda_)
			: GateBase(id_, digit_), lambda(lambda_)
		{
		}
		Phase_Bool(std::string_view reg_, double lambda_) : Phase_Bool(reg_, 0, lambda_) {}
		Phase_Bool(size_t id_, double lambda_) : Phase_Bool(id_, 0, lambda_) {}
		void operator()(std::vector<System>& state) const;
		void dag(std::vector<System>& state) const;
		//void display() const override;
	};

	struct Rot_Bool : BaseOperator, GateBase {
		using BaseOperator::operator();
		using BaseOperator::dag;

		using angle_function_t = std::function<u22_t(size_t)>;
		uint64_t mask;
		u22_t mat;
		ClassControllable
		Rot_Bool(std::string_view reg_, size_t digit_, u22_t mat)
			: GateBase(System::get(reg_), digit_), mat(mat)
		{
			mask = pow2(digit);
		}
		Rot_Bool(size_t id_, size_t digit_, u22_t mat)
			: GateBase(id_, digit_), mat(mat)
		{
			mask = pow2(digit);
		}
		Rot_Bool(std::string_view reg_, u22_t mat) : Rot_Bool(reg_, 0, mat) {
			// check the digit of the register to be 1
			if (System::size_of(reg_) != 1) {
				throw_invalid_input();
			}
		}
		Rot_Bool(size_t id_, u22_t mat) : Rot_Bool(id_, 0, mat) {
			// check the digit of the register to be 1
			if (System::size_of(id_) != 1) {
				throw_invalid_input();
			}
		}

		void operate(size_t l, size_t r, std::vector<System>& state) const;

		static bool _is_diagonal(const u22_t& data);
		void _operate_diagonal(size_t l, size_t r,
			std::vector<System>& state, const u22_t& mat) const;

		static bool _is_off_diagonal(const u22_t& data);
		void _operate_off_diagonal(size_t l, size_t r,
			std::vector<System>& state, const u22_t& mat) const;

		void _operate_general(size_t l, size_t r,
			std::vector<System>& state, const u22_t& mat) const;

		void operate_pair(size_t zero, size_t one, std::vector<System>& state) const;
		void operate_alone_zero(size_t zero, std::vector<System>& state) const;
		void operate_alone_one(size_t one, std::vector<System>& state) const;

		void operator()(std::vector<System>& state) const;
		void dag(std::vector<System>& state) const;
		//void display() const override;
#ifdef USE_CUDA
		void cu_operate_diagonal(CuSparseState& s, complex_t u00, complex_t u11) const;
		void cu_operate_off_diagonal(CuSparseState& s, complex_t u01, complex_t u10) const;
		void cu_operate_general(CuSparseState& s, complex_t u00, complex_t u01, complex_t u10, complex_t u11) const;
		void operator()(CuSparseState& s) const;
#endif
	};

	/* Flip a certain digit in a General-Purpose register. */
	struct Xgate_Bool : SelfAdjointOperator, GateBase {
		using SelfAdjointOperator::operator();
		using SelfAdjointOperator::dag;

		ClassControllable
		Xgate_Bool(std::string_view reg_, size_t digit_)
			: GateBase(System::get(reg_), digit_)
		{
		}
		Xgate_Bool(size_t id_, size_t digit_)
			: GateBase(id_, digit_)
		{
		}
		Xgate_Bool(std::string_view reg_) : Xgate_Bool(reg_, 0) {}
		Xgate_Bool(size_t id_) : Xgate_Bool(id_, 0) {}
		void operator()(std::vector<System>& state) const;
		//void display() const override;
#ifdef USE_CUDA
		void operator()(CuSparseState& state) const;
#endif
	};

	struct Ygate_Bool : SelfAdjointOperator, GateBase {
		using SelfAdjointOperator::operator();
		using SelfAdjointOperator::dag;
		using GateBase::GateBase;

		ClassControllable
		void operator()(std::vector<System>& state) const;
		//void display() const override;
		DenseMatrix<complex_t> extract_matrix();
	};

	struct Zgate_Bool : Phase_Bool
	{
		using Phase_Bool::operator();
		using Phase_Bool::dag;
		using Phase_Bool::Phase_Bool;

		// Convenience constructors for single-qubit Z gate (lambda = pi)
		Zgate_Bool(std::string_view reg_) : Phase_Bool(reg_, 0, M_PI) {}
		Zgate_Bool(size_t id_) : Phase_Bool(id_, 0, M_PI) {}

		//void display() const override;
	};

	struct Sgate_Bool : Phase_Bool
	{
		using Phase_Bool::operator();
		using Phase_Bool::dag;
		using Phase_Bool::Phase_Bool;

		// Convenience constructors for S gate (lambda = pi/2)
		Sgate_Bool(std::string_view reg_) : Phase_Bool(reg_, 0, M_PI / 2) {}
		Sgate_Bool(size_t id_) : Phase_Bool(id_, 0, M_PI / 2) {}

		//void display() const override;
	};

	struct Tgate_Bool : Phase_Bool
	{
		using Phase_Bool::operator();
		using Phase_Bool::dag;
		using Phase_Bool::Phase_Bool;

		// Convenience constructors for T gate (lambda = pi/4)
		Tgate_Bool(std::string_view reg_) : Phase_Bool(reg_, 0, M_PI / 4) {}
		Tgate_Bool(size_t id_) : Phase_Bool(id_, 0, M_PI / 4) {}

		//void display() const override;
	};

	struct RXgate_Bool : Rot_Bool
	{
		using Rot_Bool::operator();
		using Rot_Bool::dag;

		u22_t mat;
		RXgate_Bool(std::string_view reg_, size_t digit_, double angle_);
		RXgate_Bool(size_t id_, size_t digit_, double angle_);
		RXgate_Bool(std::string_view reg_, double angle_) : RXgate_Bool(reg_, 0, angle_) {}
		RXgate_Bool(size_t id_, double angle_) : RXgate_Bool(id_, 0, angle_) {}
		//void display() const override;
		inline void operator()(std::vector<System>& state) const
		{
			Rot_Bool::operator()(state);
		}
	};

	struct RYgate_Bool : Rot_Bool
	{
		using Rot_Bool::operator();
		using Rot_Bool::dag;

		u22_t mat;
		RYgate_Bool(std::string_view reg, size_t digit_, double angle_);
		RYgate_Bool(size_t id_, size_t digit_, double angle_);
		RYgate_Bool(std::string_view reg_, double angle_) : RYgate_Bool(reg_, 0, angle_) {}
		RYgate_Bool(size_t id_, double angle_) : RYgate_Bool(id_, 0, angle_) {}
		//void display() const override;
		inline void operator()(std::vector<System>& state) const
		{
			Rot_Bool::operator()(state);
		}
	};

	struct RZgate_Bool : BaseOperator, GateBase {
		using BaseOperator::operator();
		using BaseOperator::dag;

		double angle;
		ClassControllable
		RZgate_Bool(std::string_view reg_, size_t digit_, double angle_);
		RZgate_Bool(size_t id_, size_t digit_, double angle_);
		RZgate_Bool(std::string_view reg_, double angle_) : RZgate_Bool(reg_, 0, angle_) {}
		RZgate_Bool(size_t id_, double angle_) : RZgate_Bool(id_, 0, angle_) {}

		void operator()(std::vector<System>& state) const;
		//void display() const override;
	};

	struct SXgate_Bool : Rot_Bool
	{
		using Rot_Bool::operator();
		using Rot_Bool::dag;

		u22_t mat;
		SXgate_Bool(std::string_view reg_, size_t digit_);
		SXgate_Bool(size_t id_, size_t digit_);
		SXgate_Bool(std::string_view reg_) : SXgate_Bool(reg_, 0) {}
		SXgate_Bool(size_t id_) : SXgate_Bool(id_, 0) {}

		inline void operator()(std::vector<System>& state) const
		{
			Rot_Bool::operator()(state);
		}
		//void display() const override;
	};

	struct U2gate_Bool : Rot_Bool
	{
		using Rot_Bool::operator();
		using Rot_Bool::dag;

		u22_t mat;
		double phi;
		double lambda;
		U2gate_Bool(std::string_view reg_, size_t digit_, double phi, double lambda);
		U2gate_Bool(size_t id_, size_t digit_, double phi, double lambda);
		U2gate_Bool(std::string_view reg_, double phi, double lambda) : U2gate_Bool(reg_, 0, phi, lambda) {}
		U2gate_Bool(size_t id_, double phi, double lambda) : U2gate_Bool(id_, 0, phi, lambda) {}

		inline void operator()(std::vector<System>& state) const
		{
			Rot_Bool::operator()(state);
		}
		//void display() const override;
	};

	struct U3gate_Bool : Rot_Bool
	{
		using Rot_Bool::operator();
		using Rot_Bool::dag;

		double theta;
		double phi;
		double lambda;

		U3gate_Bool(std::string_view reg, size_t digit_, double theta, double phi, double lambda);
		U3gate_Bool(size_t id_, size_t digit_, double theta, double phi, double lambda);
		U3gate_Bool(std::string_view reg_, double theta, double phi, double lambda) : U3gate_Bool(reg_, 0, theta, phi, lambda) {}
		U3gate_Bool(size_t id_, double theta, double phi, double lambda) : U3gate_Bool(id_, 0, theta, phi, lambda) {}

		inline void operator()(std::vector<System>& state) const
		{
			Rot_Bool::operator()(state);
		}
		//void display() const override;
	};
}

