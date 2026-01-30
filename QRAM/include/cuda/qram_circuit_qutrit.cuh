#pragma once

#include "qram_circuit_qutrit.h"
#include "cuda_utils.cuh"

namespace qram_simulator {
	namespace qram_qutrit {
		struct CuQRAMCircuit : public QRAMCircuit {
			thrust::device_vector<memory_entry_t> memory_dev;

			CuQRAMCircuit(size_t address_sz, size_t data_sz)
				: QRAMCircuit(address_sz, data_sz)
			{
				memory_dev.resize(pow2(address_sz));
			}
			CuQRAMCircuit(size_t address_sz, size_t data_sz, const memory_t& memory_)
				: QRAMCircuit(address_sz, data_sz, memory_)
			{
				std_to_thrust_device(memory_dev, memory);
			}
			CuQRAMCircuit(size_t address_sz, size_t data_sz, memory_t&& memory_)
				: QRAMCircuit(address_sz, data_sz, std::move(memory_))
			{
				std_to_thrust_device(memory_dev, memory);
			}
			CuQRAMCircuit(const QRAMCircuit& other)
				: QRAMCircuit(other.address_size, other.data_size, other.memory)
			{
				std_to_thrust_device(memory_dev, memory);
			}

			void set_memory_random()
			{
				random_memory(memory, data_size);
			}

			void set_memory(const memory_t& new_memory)
			{
				if (new_memory.size() != pow2(address_size))
					throw_invalid_input();

				memory = new_memory;
				std_to_thrust_device(memory_dev, memory);
			}

			void set_memory(memory_t&& new_memory)
			{
				if (new_memory.size() != pow2(address_size))
					throw_invalid_input();

				memory = std::move(new_memory);
				std_to_thrust_device(memory_dev, memory);
			}
		};
	}
}