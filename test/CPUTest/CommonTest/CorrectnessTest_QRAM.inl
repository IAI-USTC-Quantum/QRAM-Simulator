
#include "qram_circuit_qutrit.h"
#include "qram_circuit_qubit.h"

template<typename QRAM_type = qram_qutrit::QRAMCircuit>
QRAM_type configure_qram(size_t addr_sz, size_t data_sz, const noise_t& noise, seed_t seed, size_t input_sz)
{
	random_engine::get_instance().set_seed(seed);

	QRAM_type qram(addr_sz, data_sz);
	qram.set_memory_random();
	qram.set_noise_models(noise);
	qram.set_input_uniform(input_sz);
	// qram.set_input_random(input_sz);

	return qram;
}

auto QRAM_compare_impl(qram_qutrit::QRAMCircuit& qram, size_t addr, size_t data, std::string version, seed_t seed)
{
	random_engine::get_instance().set_seed(seed);

	if (version == qram_qutrit::QRAMCircuit::FULL_VER) {
		qram.run_full();
	}
	else if (version == qram_qutrit::QRAMCircuit::NORMAL_VER) {
		qram.run_normal();
	}
	else
		throw_invalid_input();

	double fidelity = qram.sample_and_get_fidelity();
	return fidelity;
}

auto QRAM_compare_test(size_t addr, size_t data, seed_t seed, const noise_t& noise, int trials, size_t input_sz)
{
	for (int it = 0; it < trials; ++it)
	{
		random_engine::get_instance().set_seed(seed + it);
		auto runseed = random_engine::get_instance().reseed();
		fmt::print("{} / {} (seed={})\n", it, trials, runseed);

		auto qram_old = configure_qram(addr, data, noise, runseed, input_sz);
		auto old_fid = QRAM_compare_impl(qram_old, addr, data, "full", runseed);

		auto qram_new = configure_qram(addr, data, noise, runseed, input_sz);
		auto new_fid = QRAM_compare_impl(qram_new, addr, data, "normal", runseed);

		if (!ignorable(new_fid - old_fid))
		{
			TEST_FAIL("Fidelity different between full and normal.");
		}
		fmt::print("pass, fid_full={:.5f}, fid_normal={:.5f}\n", old_fid, new_fid);
	}
}

auto QRAM_compare_test()
{
	size_t addr = 10;
	size_t data = 3;
	seed_t seed = 191781;
	noise_t noise = {
		{OperationType::Depolarizing,1e-4},
		{OperationType::Damping,1e-4}
	};
	int trials = 100;
	size_t input_sz = 1000;

	QRAM_compare_test(addr, data, seed, noise, trials, input_sz);
}

/* Qubit architecture, full (no-pruning) simulation: ground truth checks */
auto QRAMQubit_full_impl_1shot(size_t addr_sz, size_t data_sz,
	const noise_t& noise, seed_t seed, size_t input_sz)
{
	auto qram = configure_qram<qram_qubit::QRAMCircuit>(addr_sz, data_sz, noise, seed, input_sz);
	random_engine::get_instance().set_seed(seed);
	qram.run_full();
	return qram.sample_and_get_fidelity();
}

auto QRAMQubit_FullCorrectnessTest()
{
	/* Noiseless: full simulation must reproduce the ideal QRAM exactly */
	for (size_t addr : { 2, 3, 4 })
	{
		for (size_t data : { 1, 2 })
		{
			noise_t noise_free;
			auto fid = QRAMQubit_full_impl_1shot(addr, data, noise_free, 233, pow2(addr + data));
			fmt::print("qubit full (noiseless): n={} k={} fid={:.15f}\n", addr, data, fid);
			if (!ignorable(fid - 1.0))
			{
				TEST_FAIL("Qubit full simulation without noise should give fidelity 1.");
			}
		}
	}

	/* Noisy: deterministic under the same seed, fidelity within [0, 1] */
	noise_t noise = {
		{ OperationType::Depolarizing, 1e-3 },
		{ OperationType::Damping, 1e-4 }
	};
	for (int it = 0; it < 20; ++it)
	{
		seed_t seed = 191781 + it;
		auto fid1 = QRAMQubit_full_impl_1shot(4, 2, noise, seed, 16);
		auto fid2 = QRAMQubit_full_impl_1shot(4, 2, noise, seed, 16);
		fmt::print("qubit full (noisy): seed={} fid={:.5f}\n", seed, fid1);
		if (std::isnan(fid1) || fid1 < 0 || fid1 > 1 + epsilon)
		{
			TEST_FAIL("Qubit full simulation fidelity out of range.");
		}
		if (fid1 != fid2)
		{
			TEST_FAIL("Qubit full simulation is not deterministic under the same seed.");
		}
	}
}
