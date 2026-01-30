#include "state_preparation.h"

namespace qram_simulator {
	namespace state_prep {
	}

	namespace state_preparation_demo {

		void SparseStateDemo::clear_state()
		{
			system_states.clear();
			system_states.emplace_back();
		}

		void SparseStateDemo::sort_state()
		{
			std::sort(system_states.begin(), system_states.end());
		}

		std::string SparseStateDemo::to_string() const
		{
			std::vector<char> buf;
			fmt::format_to(std::back_inserter(buf), "QRAMState = \n");
			for (auto& state : system_states)
				fmt::format_to(std::back_inserter(buf), "{}\n", state.to_string());

			return { buf.data(), buf.size() };
		}

		void SparseStateDemo::run()
		{
			profiler _("StatePrep::run");

			Xgate_Bool("addr_parent", 0)(system_states);
			for (size_t k = 0; k < addr_size - 1; ++k) {
				Mult_UInt_ConstUInt("addr_parent", 2, "addr_child")(system_states);
				QRAMLoad(qram, "addr_parent", "data_parent")(system_states);
				QRAMLoad(qram, "addr_child", "data_child")(system_states);
				Div_Sqrt_Arccos_Int_Int("data_child", "data_parent", "div_result")(system_states);
				CondRot_Rational_Bool("div_result", "temp_bit")(system_states);
				Div_Sqrt_Arccos_Int_Int("data_child", "data_parent", "div_result")(system_states);
				QRAMLoad(qram, "addr_child", "data_child")(system_states);
				QRAMLoad(qram, "addr_parent", "data_parent")(system_states);
				Mult_UInt_ConstUInt("addr_parent", 2, "addr_child")(system_states);
				ShiftLeft("addr_parent", 1)(system_states);
				Swap_Bool_Bool("temp_bit", 0, "addr_parent", 0)(system_states);
			}
			ShiftLeft("addr_parent", 1)(system_states);
			Assign("addr_parent", "addr_child")(system_states);
			Xgate_Bool("addr_child", 0)(system_states);
			QRAMLoad(qram, "addr_parent", "data_parent")(system_states);
			QRAMLoad(qram, "addr_child", "data_child")(system_states);
			GetRotateAngle_Int_Int("data_parent", "data_child", "div_result")(system_states);
			CondRot_Rational_Bool("div_result", "temp_bit")(system_states);
			GetRotateAngle_Int_Int("data_parent", "data_child", "div_result")(system_states);
			QRAMLoad(qram, "addr_child", "data_child")(system_states);
			QRAMLoad(qram, "addr_parent", "data_parent")(system_states);
			Xgate_Bool("addr_child", 0)(system_states);
			Assign("addr_parent", "addr_child")(system_states);
			Swap_Bool_Bool("temp_bit", 0, "addr_parent", 0)(system_states);
			Xgate_Bool("addr_parent", addr_size)(system_states);

			SortByKey("addr_parent")(system_states);
		}

		void StatePreparation::random_distribution()
		{
			size_t sz = pow2(qubit_number);
			size_t data_max = pow2(data_range);
			dist.resize(sz, 0);

			std::uniform_int_distribution<size_t> ud(0, data_max - 1);

			for (size_t i = 0; i < sz; ++i)
			{
				size_t data = ud(random_engine::get_engine());
				if (data >= pow2(data_range - 1))
					data = pow2(data_size) - (pow2(data_range) - data);
				dist[i] = data;
			}
		}

		void StatePreparation::show_distribution()
		{
			size_t sum = 0;
			for (auto a : dist) {
				sum += get_complement(a, data_size) * get_complement(a, data_size);
			}
			double A = sqrt(sum);
			fmt::print("index | original | amplitude\n");
			for (size_t i = 0; i < dist.size(); ++i)
			{
				fmt::print("{:^5d} | {:^8d} | {:f}\n", i,
					get_complement(dist[i], data_size),
					get_complement(dist[i], data_size) / A);
			}
		}

		std::vector<double> StatePreparation::get_real_dist()
		{
			size_t sum = 0;
			std::vector<double> real_dist(dist.size(), 0);
			for (auto a : dist) {
				sum += get_complement(a, data_size) * get_complement(a, data_size);
			}
			double A = sqrt(sum);
			for (size_t i = 0; i < dist.size(); ++i)
			{
				real_dist[i] = get_complement(dist[i], data_size) / A;
			}
			return real_dist;
		}

		void StatePreparation::make_tree()
		{
			size_t dist_sz = dist.size();
			std::vector<size_t> temp_tree;
			temp_tree = dist;
			tree.clear();
			tree.push_back(0);
			do
			{
				std::vector<size_t> temp;
				temp.reserve(dist_sz);

				for (size_t i = 0; i < dist_sz; i += 2)
				{
					if (dist_sz == dist.size())
						temp.push_back(
							get_complement(temp_tree[i], data_size) * get_complement(temp_tree[i], data_size) +
							get_complement(temp_tree[i + 1], data_size) * get_complement(temp_tree[i + 1], data_size)
						);
					else
						temp.push_back(temp_tree[i] + temp_tree[i + 1]);
				}
				temp.insert(temp.end(), temp_tree.begin(), temp_tree.end());
				temp_tree = std::move(temp);

			} while ((dist_sz >>= 1) > 1);
			tree.insert(tree.end(), temp_tree.begin(), temp_tree.end());
		}

		void StatePreparation::show_tree()
		{
			size_t begin = 1;
			for (size_t n = 0; n <= qubit_number; ++n) {
				for (size_t i = 0; i < pow2(n); ++i)
				{
					auto showdata = tree[begin + i];
					if (n == qubit_number)
						showdata = get_complement(tree[begin + i], data_size);
					fmt::print("({}) {:^4d} ", begin + i, showdata);
				}
				begin += pow2(n);
				fmt::print("\n");
			}
		}

		void StatePreparation::make_qram()
		{
			qram = new qram_qutrit::QRAMCircuit(qubit_number + 1, data_size);
		}

		void StatePreparation::set_qram()
		{
			qram->set_memory(tree);
		}

		void StatePreparation::set_noise(const noise_t& noise)
		{
			qram->set_noise_models(
				noise
			);
		}

		void StatePreparation::run()
		{
			sparse_state.qram = qram;
			sparse_state.run();
		}

		double StatePreparation::get_fidelity() const
		{
			complex_t fid = 0;
			size_t sum = 0;
			for (auto a : dist) {
				sum += get_complement(a, data_size) * get_complement(a, data_size);
			}
			double A = sqrt(sum);
			auto addr_parent = System::get("addr_parent");
			for (auto& state : sparse_state.system_states)
			{
				double t = get_complement(dist[state.get(addr_parent).value], data_size) / A;
				complex_t ampl = state.amplitude;
				fid += (t * ampl);
				check_nan(fid.real());
			}
			return fid.real() * fid.real() + fid.imag() * fid.imag();
		}

		double StatePreparation::get_fidelity_show() const
		{
			complex_t fid = 0;
			size_t sum = 0;
			for (auto a : dist) {
				sum += get_complement(a, data_size) * get_complement(a, data_size);
			}
			double A = sqrt(sum);
			fmt::print("index | original | amplitude | result \n");
			auto addr_parent = System::get("addr_parent");
			for (auto& state : sparse_state.system_states)
			{
				size_t i = state.get(addr_parent).value;
				double t = get_complement(dist[state.get(addr_parent).value], data_size) / A;
				complex_t ampl = state.amplitude;
				fid += (t * ampl);
				fmt::print("{:^5d} | {:^8d} | {:f} | {} \n", i,
					get_complement(dist[i], data_size),
					get_complement(dist[i], data_size) / A,
					ampl);
				check_nan(fid.real());
			}
			return fid.real() * fid.real() + fid.imag() * fid.imag();
		}
	}
}