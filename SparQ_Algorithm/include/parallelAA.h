#pragma once

#if false

#include "hamiltonian_simulation.h"
#include "matrix.h"

namespace qram_simulator {
	namespace parallelAA {
		class AlgorithmA
		{
		public:
			// AlgorithmA A0 = AlgorithmA("rank1", "st", "flag", qram0, 0, angle_func0);
			AlgorithmA(std::string index, std::string st, std::string rotat, qram_qutrit::QRAMCircuit& qram_, int value, std::function<u22_t(size_t)> angle_function_) :
				reg_index_t(System::get(index)), reg_st_t(System::get(st)), reg_rotat_t(System::get(rotat)), qram(qram_), index_value(value),
				name_index(index), name_rotat(rotat), name_st(st), angle_function(angle_function_)
			{
			}
			~AlgorithmA() {}
			inline bool compare_equal_value(const System& a, int out_id1, int val)
			{
				if (a.get(out_id1).value == val) return true;
				else return false;

			}
			inline void LoadValue(std::vector<System>& states)
			{
				QRAMLoad(&qram, name_index, name_st)
					.conditioned_by_nonzeros("compareFlag")(states);
			}
			inline void CondRotat(std::vector<System>& states)
			{
				std::vector<System> c_states;
				for (auto ite = states.begin(); ite != states.end(); ite++)
				{
					if (ite->get(System::get("compareFlag")).value == true)
					{
						c_states.insert(c_states.end(), std::make_move_iterator(ite), std::make_move_iterator(ite + 1));


					}
				}
				states.erase(std::remove_if(states.begin(), states.end(),
					[&](System s) {if (s.get(System::get("compareFlag")).value == true) { return true; } else return false; }),
					states.end());
				CondRot_General_Bool(name_st, name_rotat, angle_function)(c_states);
				states.insert(states.end(), c_states.begin(), c_states.end());
				c_states.clear();
				ClearZero()(states);
			}

			inline void FlipCompareFlag(std::vector<System>& states)
			{

				for (int i = 0; i < states.size(); i++)
				{
					if (compare_equal_value(states[i], reg_index_t, index_value))
					{
						states[i].get(System::get("compareFlag")).flip(0);
					}
				}
			}
			//�ҵ�compareflagΪtrue����flagΪfalse���Ǹ�state�����
			inline void calculateT(std::vector<System>& states)
			{
				for (int i = 0; i < states.size(); ++i)
				{
					if (states[i].get(System::get("compareFlag")).value == true && states[i].get(System::get("flag")).value == false)
					{
						aim_amp = states[i].amplitude;
						double real = aim_amp.real();
						theta = asin(real * 2);
						T = pi / (4 * theta);
					}
				}
			}
			inline void operator()(std::vector<System>& states)
			{
				FlipCompareFlag(states);
				LoadValue(states);
				CondRotat(states);
				calculateT(states);
				FlipCompareFlag(states);
			}

			std::string name_index;
			std::string name_st;
			std::string name_rotat;
			int reg_index_t;
			int reg_st_t;
			int reg_rotat_t;
			int index_value;
			qram_qutrit::QRAMCircuit qram;
			std::function<u22_t(size_t)> angle_function;

			std::complex<double> aim_amp;
			double theta;
			unsigned int T;
		};

		class AlgorithmA_re
		{
		public:
			AlgorithmA_re(std::string index, std::string st, std::string rotat, qram_qutrit::QRAMCircuit& qram_, int value, std::function<u22_t(size_t)> angle_function_) :
				qram(qram_), index_value(value), name_index(index), name_rotat(rotat), name_st(st), angle_function(angle_function_)
			{
				reg_index_t = (System::get(index));
				reg_st_t = (System::get(st));
				reg_rotat_t = (System::get(rotat));
			}
			~AlgorithmA_re() {}
			inline bool compare_equal_value(const System& a, int out_id1, int val)
			{
				if (a.get(out_id1).value == val) return true;
				else return false;
			}
			inline void LoadValue(std::vector<System>& states)
			{
				QRAMLoad(&qram, name_index, name_st)
					.conditioned_by_nonzeros("compareFlag")(states);
			}
			inline void CondRotat(std::vector<System>& states)
			{
				/*
				for (int i = 0; i < states.size(); ++i)
				{
					if (states[i].get(System::get("compareFlag")).value == true)
					{
						CondRot_General_Bool(name_st, name_rotat, angle_function)(states);
					}
				}
				*/
				std::vector<System> c_states;

				for (auto ite = states.begin(); ite != states.end(); ite++)
				{
					if (ite->get(System::get("compareFlag")).value == true)
					{
						c_states.insert(c_states.end(), std::make_move_iterator(ite), std::make_move_iterator(ite + 1));


					}
				}
				states.erase(std::remove_if(states.begin(), states.end(),
					[&](System s) {if (s.get(System::get("compareFlag")).value == true) { return true; } else return false; }),
					states.end());
				CondRot_General_Bool(name_st, name_rotat, angle_function)(c_states);
				states.insert(states.end(), c_states.begin(), c_states.end());
				c_states.clear();
			}
			inline void FlipCompareFlag(std::vector<System>& states)
			{

				for (int i = 0; i < states.size(); i++)
				{
					if (compare_equal_value(states[i], reg_index_t, index_value))
					{
						states[i].get(System::get("compareFlag")).flip(0);
					}
				}
			}
			inline void operator()(std::vector<System>& states)
			{
				//System::add_register("compareFlag", Boolean, 1);
				FlipCompareFlag(states);
				CondRotat(states);
				LoadValue(states);
				FlipCompareFlag(states);
				//RemoveRegister("compareFlag")(states);

			}

			std::string name_index;
			std::string name_st;
			std::string name_rotat;
			int reg_index_t;
			int reg_st_t;
			int reg_rotat_t;
			int index_value;
			qram_qutrit::QRAMCircuit qram;
			std::function<u22_t(size_t)> angle_function;
		};

		class AlgorithmQ
		{
		public:
			// AlgorithmA A0 = AlgorithmA("rank1", "st", "flag", qram0, 0, angle_func0);
			//AlgorithmQ Q0 = AlgorithmQ(AlgotirhmA& A, AlgorithmA_re& Are)
			AlgorithmQ(AlgorithmA& A_, AlgorithmA_re& Are_) :A(A_), Are(Are_) {
				name_index = A_.name_index;
				name_st = A_.name_st;
				name_rotat = A_.name_rotat;
				index_value = A_.index_value;
			}
			~AlgorithmQ() {}

			inline bool compare_equal_value(const System& a, int out_id1, int val)
			{
				if (a.get(out_id1).value == val) return true;
				else return false;

			}
			inline void FlipCompareFlag(std::vector<System>& states)
			{
				for (int i = 0; i < states.size(); i++)
				{
					if (compare_equal_value(states[i], System::get(name_index), index_value))
					{
						states[i].get(System::get("compareFlag")).flip(0);
					}
				}
			}
			inline void Negative_S_X(std::vector<System>& states)
			{
				//S_X
				for (int i = 0; i < states.size(); i++)
				{
					if (compare_equal_value(states[i], System::get(name_index), index_value))
					{
						if (states[i].get(System::get(name_rotat)).value == true)
						{
							states[i].amplitude *= -1;
						}
					}
				}
			}
			inline void S_0(std::vector<System>& states)
			{
				//System::add_register("compareFlag", Boolean, 1);
				FlipCompareFlag(states);
				ZeroConditionalPhaseFlip(std::vector{ (int)System::get(name_st),(int)System::get(name_rotat) }).conditioned_by_nonzeros("compareFlag")(states);
				FlipCompareFlag(states);
				//RemoveRegister("compareFlag")(states);
			}
			inline void operator()(std::vector<System>& states, unsigned int n)
			{
				for (int i = 0; i < n; ++i)
				{
					Negative_S_X(states);
					Are(states);
					//S_0
					S_0(states);
					A(states);
				}
			}

			std::string name_index;
			std::string name_st;
			std::string name_rotat;
			int index_value;
			AlgorithmA A;
			AlgorithmA_re Are;
		};

		class AlgorithmEST
		{
		public:
			AlgorithmEST(std::string ancilla, AlgorithmQ Q_) : name_ancilla(ancilla), Q(Q_)
			{
				name_index = Q_.name_index;
				name_st = Q_.name_st;
				name_rotat = Q_.name_rotat;
				index_value = Q_.index_value;
			}
			~AlgorithmEST() {}
			inline bool compare_equal_value(const System& a, int out_id1, int val)
			{
				if (a.get(out_id1).value == val) return true;
				else return false;

			}
			inline void FlipCompareFlag(std::vector<System>& states)
			{

				for (int i = 0; i < states.size(); i++)
				{
					if (compare_equal_value(states[i], System::get(name_index), index_value))
					{
						states[i].get(System::get("compareFlag")).flip(0);
					}
				}
			}
			inline void Apply_Multiple_Q(std::vector<System>& states)
			{
				std::vector<System> tmp;
				int maxNum = 1 << (System::size_of(name_ancilla));

				for (int i = 0; i < maxNum; ++i)
				{
					detach(states, tmp, name_ancilla, maxNum - i - 1);
					Q(tmp, i);
					//printf("����Q_%d %d����st and rotat��\n", i, i);
					merge(states, tmp);
				}
			}
			inline void detach(std::vector<System>& states, std::vector<System>& c_states, std::string regName, int value)
			{
				for (auto ite = states.begin(); ite != states.end(); ite++)
				{
					if (ite->get(System::get(regName)).value == value)
					{
						c_states.insert(c_states.end(), std::make_move_iterator(ite), std::make_move_iterator(ite + 1));
					}
				}
				states.erase(std::remove_if(states.begin(), states.end(),
					[&](System s) {if (s.get(System::get(regName)).value == value) { return true; } else return false; }),
					states.end());
			}
			inline void merge(std::vector<System>& states, std::vector<System>& c_states)
			{
				states.insert(states.end(), c_states.begin(), c_states.end());
				c_states.clear();
				ClearZero()(states);
			}
			inline void operator()(std::vector<System>& states)
			{

				std::vector<System> c_states;

				FlipCompareFlag(states);
				detach(states, c_states, "compareFlag", true);
				FlipCompareFlag(states);
				FlipCompareFlag(c_states);

				QFT(System::get(name_ancilla))(c_states);
				(StatePrint(StatePrintDisplay::Detail))(c_states);
				Apply_Multiple_Q(c_states);
				std::cout << "after calling Apply_Multiple_Q(c_states);\n ";
				(StatePrint(StatePrintDisplay::Detail))(c_states);
				inverseQFT(System::get(name_ancilla))(c_states);
				std::cout << "after calling iQFT...\n";

				(StatePrint(StatePrintDisplay::Detail))(c_states);

				merge(states, c_states);

			}
			std::string name_index;
			std::string name_st;
			std::string name_rotat;
			std::string name_ancilla;
			AlgorithmQ Q;
			int index_value;
		};
	}
}

#endif