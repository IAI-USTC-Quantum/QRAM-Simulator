
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


auto QRAMQutrit_FidelityTestImpl_1shot(size_t addr_sz, size_t data_sz, QRAMInputGenerator& gen,
	const noise_t& noise, size_t input_size, std::string version, size_t addr, size_t data)
{
	std::vector<System> state;
	gen.generate_input(state);

	/****** QRAM Initialize ******/
	qram_qutrit::QRAMCircuit qram = qram_qutrit::QRAMCircuit(addr_sz, data_sz);
	qram.set_memory_random();
	qram.set_noise_models(noise);

	QRAMLoad::version = version;
	QRAMLoad(&qram, addr, data)(state);
	QRAMLoad::version = "noisefree";
	QRAMLoad(&qram, addr, data)(state);
	complex_t fidelity = 0;

	for (auto& s : state)
	{
		auto p = std::make_pair(s.get(addr).value, s.get(data).value);
		if (std::find(gen.unique_set.begin(), gen.unique_set.end(), p)
			!= gen.unique_set.end())
		{
			fidelity += std::conj(s.amplitude) / std::sqrt(gen.input_sz);
		}
	}
	return abs_sqr(fidelity);
}


auto QRAMQutrit_FidelityTestImpl(size_t addr_sz, size_t data_sz,
	seed_t seed, const noise_t& noise, size_t shots, size_t input_size)
{
	size_t max_input_size;

	if (addr_sz + data_sz >= 32)
		max_input_size = pow2(32);
	else
		max_input_size = pow2(addr_sz + data_sz);

	input_size = std::min(input_size, max_input_size);

	auto addr = System::add_register("addr", UnsignedInteger, addr_sz);
	auto data = System::add_register("data", UnsignedInteger, data_sz);

	QRAMInputGenerator gen(addr_sz, data_sz, input_size, addr, data);

	double sum_fidelity_normal = 0;
	double sum_fidelity_full = 0;

	for (size_t i = 0; i < shots; ++i)
	{
		random_engine::get_instance().set_seed(seed + i);
		auto fidelity_normal = QRAMQutrit_FidelityTestImpl_1shot(addr_sz, data_sz, gen, noise, input_size, "normal", addr, data);

		random_engine::get_instance().set_seed(seed + i);
		auto fidelity_full = QRAMQutrit_FidelityTestImpl_1shot(addr_sz, data_sz, gen, noise, input_size, "full", addr, data);

		fmt::print("Fidelity normal/full = {}, {}\n", fidelity_normal, fidelity_full); 
		if (std::isnan(fidelity_normal) || std::isnan(fidelity_full))
		{
			TEST_FAIL("Fidelity is NaN");
		}	
		if (std::abs(fidelity_normal - fidelity_full) > 1e-6)
		{
			TEST_FAIL("Fidelity is not equal for normal and full versions");
		}
		sum_fidelity_normal += fidelity_normal;
		sum_fidelity_full += fidelity_full;
	}

	if (std::abs(sum_fidelity_normal - sum_fidelity_full) > 1e-6)
	{
		TEST_FAIL("Fidelity is not equal for normal and full versions");
	}
}

auto QRAMQutrit_FidelityTest()
{
	size_t addr_sz = 5;
	size_t data_sz = 3;
	size_t shots = 100;
	size_t input_size = 100;
	seed_t seed = 1234;

	noise_t noise = {
		{ OperationType::Depolarizing, 1e-3},
		{ OperationType::Damping, 1e-4 }
	};

	QRAMQutrit_FidelityTestImpl(addr_sz, data_sz, seed, noise, shots, input_size);
}

