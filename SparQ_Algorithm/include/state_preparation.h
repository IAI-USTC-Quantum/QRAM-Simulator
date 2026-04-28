#pragma once
#include "state_manipulator.h"
#include "simple_quantum_simulator.h"
#include "sparse_state_simulator.h"
#include "global_macros.h"

namespace qram_simulator {
	namespace state_prep {
		struct State_Prep_via_QRAM : BaseOperator
		{
			std::string work_qubit;
			size_t addr_size;
			size_t data_size;
			size_t rational_size;
			qram_qutrit::QRAMCircuit* qram;

			ClassControllable

			State_Prep_via_QRAM(qram_qutrit::QRAMCircuit* qram_,
				std::string_view work_qubit_,
				size_t dsz,
				size_t rsz) : work_qubit(work_qubit_), addr_size(System::size_of(work_qubit)), data_size(dsz), rational_size(rsz), qram(qram_)
			{
				//QRAMLoad::version = qram_version;
			};

			template<typename Ty>
			void impl(Ty& state) const
			{
				profiler _("State_Prep_via_QRAM");

				AddRegister("addr_parent", UnsignedInteger, addr_size + 1)(state);
				AddRegister("addr_child", UnsignedInteger, addr_size + 1)(state);
				AddRegister("data_parent", SignedInteger, data_size)(state);
				AddRegister("data_child", SignedInteger, data_size)(state);
				AddRegister("div_result", Rational, rational_size)(state);

				size_t n_digit = System::size_of("div_result");
				auto func = [n_digit] HOST_DEVICE(uint64_t value) { return make_func(value, n_digit); };

				for (size_t k = 0; k < addr_size; ++k) {
					auto target = SplitRegister(work_qubit, "rotation", 1)(state);
					std::get<1>(System::name_register_map[System::get("rotation")]) = Boolean;
					Add_ConstUInt_InPlace("addr_parent", pow2(k) - 1)(state);
					Add_UInt_UInt_InPlace(work_qubit, "addr_parent")(state);
					Mult_UInt_ConstUInt("addr_parent", 2, "addr_child")(state);
					Xgate_Bool("addr_child", 0)(state);
					if (k != addr_size - 1)
					{
						QRAMLoad(qram, "addr_parent", "data_parent")(state);
						QRAMLoad(qram, "addr_child", "data_child")(state);
						Div_Sqrt_Arccos_Int_Int("data_child", "data_parent", "div_result")(state);
						{
							profiler _("StatePrep::CondRot");
							CondRot_General_Bool("div_result", "rotation", func)(state);
						}
						ClearZero()(state);
						Div_Sqrt_Arccos_Int_Int("data_child", "data_parent", "div_result")(state);
						QRAMLoad(qram, "addr_parent", "data_parent")(state);
						QRAMLoad(qram, "addr_child", "data_child")(state);
					}
					else
					{
						ShiftLeft_InPlace("addr_parent", 1)(state);
						Xgate_Bool("addr_parent", 0)(state);
						Add_ConstUInt_InPlace("addr_child", 1)(state);
						QRAMLoad(qram, "addr_parent", "data_parent")(state);
						QRAMLoad(qram, "addr_child", "data_child")(state);
						GetRotateAngle_Int_Int("data_parent", "data_child", "div_result")(state);
						{
							profiler _("StatePrep::CondRot");
							CondRot_General_Bool("div_result", "rotation", func)(state);
						}
						ClearZero()(state);
						GetRotateAngle_Int_Int("data_parent", "data_child", "div_result")(state);
						QRAMLoad(qram, "addr_parent", "data_parent")(state);
						QRAMLoad(qram, "addr_child", "data_child")(state);
						Add_ConstUInt_InPlace("addr_child", 1).dag(state);
						Xgate_Bool("addr_parent", 0)(state);
						ShiftRight_InPlace("addr_parent", 1)(state);
					}
					Xgate_Bool("addr_child", 0)(state);
					Mult_UInt_ConstUInt("addr_parent", 2, "addr_child")(state);
					Add_UInt_UInt_InPlace(work_qubit, "addr_parent").dag(state);
					Add_ConstUInt_InPlace("addr_parent", pow2(k) - 1).dag(state);
					//StatePrint(0, 10)(state);
					CombineRegister(work_qubit, "rotation")(state);
					ShiftLeft_InPlace(work_qubit, 1)(state);
				}
				ShiftRight_InPlace(work_qubit, 1)(state);
				RemoveRegister("addr_parent")(state);
				RemoveRegister("data_parent")(state);
				RemoveRegister("addr_child")(state);
				RemoveRegister("data_child")(state);
				RemoveRegister("div_result")(state);

				ClearZero()(state);
			}

			template<typename Ty>
			void impl_dag(Ty& state) const
			{
				profiler _("State_Prep_via_QRAM::dag");

				AddRegister("addr_parent", UnsignedInteger, addr_size + 1)(state);
				AddRegister("addr_child", UnsignedInteger, addr_size + 1)(state);
				AddRegister("data_parent", SignedInteger, data_size)(state);
				AddRegister("data_child", SignedInteger, data_size)(state);
				AddRegister("div_result", Rational, rational_size)(state);

				size_t n_digit = System::size_of("div_result");
				auto func = [n_digit] HOST_DEVICE(uint64_t value) { return make_func_inv(value, n_digit); };

				ShiftLeft_InPlace(work_qubit, 1)(state);
				for (size_t k = 0; k <= addr_size - 1; ++k) {
					ShiftRight_InPlace(work_qubit, 1)(state);
					auto target = SplitRegister(work_qubit, "rotation", 1)(state);
					std::get<1>(System::name_register_map[System::get("rotation")]) = Boolean;
					Add_ConstUInt_InPlace("addr_parent", pow2(addr_size - 1 - k) - 1)(state);
					Add_UInt_UInt_InPlace(work_qubit, "addr_parent")(state);
					Mult_UInt_ConstUInt("addr_parent", 2, "addr_child")(state);
					Xgate_Bool("addr_child", 0)(state);
					if (k != 0)
					{
						QRAMLoad(qram, "addr_parent", "data_parent")(state);
						QRAMLoad(qram, "addr_child", "data_child")(state);
						Div_Sqrt_Arccos_Int_Int("data_child", "data_parent", "div_result")(state);
						{
							profiler _("StatePrep::CondRot::dag");
							// size_t original_size = state.size();
							// fmt::print("stateprep original_size {}\n", original_size);
							CondRot_General_Bool("div_result", "rotation", func)(state);
						}
						ClearZero()(state);
						Div_Sqrt_Arccos_Int_Int("data_child", "data_parent", "div_result")(state);
						QRAMLoad(qram, "addr_parent", "data_parent")(state);
						QRAMLoad(qram, "addr_child", "data_child")(state);
					}
					else
					{
						ShiftLeft_InPlace("addr_parent", 1)(state);
						Xgate_Bool("addr_parent", 0)(state);
						Add_ConstUInt_InPlace("addr_child", 1)(state);
						QRAMLoad(qram, "addr_parent", "data_parent")(state);
						QRAMLoad(qram, "addr_child", "data_child")(state);
						GetRotateAngle_Int_Int("data_parent", "data_child", "div_result")(state);
						{
							profiler _("StatePrep::CondRot::dag");
							// size_t original_size = state.size();
							// fmt::print("stateprep original_size {}\n", original_size);
							CondRot_General_Bool("div_result", "rotation", func)(state);
						}
						ClearZero()(state);
						GetRotateAngle_Int_Int("data_parent", "data_child", "div_result")(state);
						QRAMLoad(qram, "addr_parent", "data_parent")(state);
						QRAMLoad(qram, "addr_child", "data_child")(state);
						Add_ConstUInt_InPlace("addr_child", 1).dag(state);
						Xgate_Bool("addr_parent", 0)(state);
						ShiftRight_InPlace("addr_parent", 1)(state);
					}
					Xgate_Bool("addr_child", 0)(state);
					Mult_UInt_ConstUInt("addr_parent", 2, "addr_child")(state);
					Add_UInt_UInt_InPlace(work_qubit, "addr_parent").dag(state);
					Add_ConstUInt_InPlace("addr_parent", pow2(addr_size - 1 - k) - 1).dag(state);
					CombineRegister(work_qubit, "rotation")(state);

				}

				RemoveRegister("addr_parent")(state);
				RemoveRegister("data_parent")(state);
				RemoveRegister("addr_child")(state);
				RemoveRegister("data_child")(state);
				RemoveRegister("div_result")(state);

				ClearZero()(state);
			}

			COMPOSITE_OPERATION
		};

	}

	namespace state_preparation_demo {

		struct SparseStateDemo
		{
			size_t addr_size;
			size_t data_size;
			size_t rational_size;
			std::vector<System> system_states;
			qram_qutrit::QRAMCircuit* qram;
			std::string qram_version;
			SparseStateDemo(size_t asz, size_t dsz, size_t rsz, std::string qram_version_)
				: addr_size(asz), data_size(dsz), rational_size(rsz), qram_version(qram_version_)
			{
				System::add_register("addr_parent", UnsignedInteger, asz + 1);
				System::add_register("addr_child", UnsignedInteger, asz + 1);
				System::add_register("data_parent", SignedInteger, dsz);
				System::add_register("data_child", SignedInteger, dsz);
				System::add_register("temp_bit", Boolean, 1);
				System::add_register("div_result", Rational, rsz);
				system_states.emplace_back();

				QRAMLoad::version = qram_version;
			}
			void clear_state();
			void sort_state();
			std::string to_string() const;

			void run();
		};

		class StatePreparation
		{
		public:
			SparseStateDemo sparse_state;
			size_t qubit_number;
			size_t data_size;
			size_t data_range;
			std::vector<size_t> dist;
			std::vector<size_t> tree;
			qram_qutrit::QRAMCircuit* qram;
			std::string qram_version;

			StatePreparation(size_t qn, size_t data_sz, size_t data_range_, std::string qram_version_)
				: qubit_number(qn), data_size(data_sz), data_range(data_range_),
				sparse_state(qn, data_sz, std::min(size_t(50), data_sz * 2), qram_version_),
				qram_version(qram_version_)
			{
			}

			void random_distribution();
			void show_distribution();
			std::vector<double> get_real_dist();

			void make_tree();
			void show_tree();
			void make_qram();
			void set_qram();
			void set_noise(const noise_t& noise);
			double get_fidelity() const;
			double get_fidelity_show() const;
			inline void print_state() {
				StatePrint(0 | Detail)(sparse_state.system_states);
			}
			void run();
			inline void clear_state() { sparse_state.clear_state(); }
		};

	} // namespace state_preparation
} // namespace qram_simulator