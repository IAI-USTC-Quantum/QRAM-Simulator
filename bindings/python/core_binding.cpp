// Thin pybind11 binding layer over the QRAM-Simulator C++ core.
//
// Exports QRAMCircuit for both QRAM architectures (qubit / qutrit), the
// full-amplitude bridge QRAMFullAmp, the timing & noise scheduler
// TimeStep/TimeSlices/Operation(Pack), and global random-seed control, so
// external Python libraries (e.g. SparQSim) and scripted experiments can
// drive the full "construct -> set memory -> set noise -> run -> fidelity"
// workflow directly.

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4996)
#endif

#include "pybind11/pybind11.h"
#include "pybind11/stl.h"
#include "pybind11/complex.h"

#include "qram_circuit_qubit.h"
#include "qram_circuit_qutrit.h"
#include "state_manipulator.h"
#include "time_step.h"
#include "random_engine.h"

#ifdef _MSC_VER
#pragma warning(pop)
#endif

namespace py = pybind11;
using namespace qram_simulator;

PYBIND11_MODULE(_core, m)
{
	m.doc() = R"doc(
Python bindings for the QRAM-Simulator C++ core.

Provides branch-based sparse simulation of both QRAM architectures
(QRAMCircuitQubit / QRAMCircuitQutrit), the full-amplitude state-vector
bridge (QRAMFullAmp), the timing & noise scheduler (TimeStep), and global
random-seed control (set_seed / get_seed).

Example:

    from qram_simulator import QRAMCircuitQubit, OperationType, set_seed

    set_seed(42)
    qram = QRAMCircuitQubit(4, 2)
    qram.set_memory_random()
    qram.set_noise_models({OperationType.Depolarizing: 1e-3})
    qram.set_input_uniform(100)
    qram.run_normal()
    print(qram.sample_and_get_fidelity())
)doc";

	// ---- Global random-seed control (random_engine singleton) ----
	m.def("set_seed",
		[](long long seed) { random_engine::set_seed(static_cast<seed_t>(seed)); },
		py::arg("seed"),
		"Set the seed of the global random engine (MT19937-64 singleton).\n"
		"QRAMCircuit memory generation, input sampling and output sampling all\n"
		"use this engine; fixing the seed reproduces the whole simulation chain.");
	m.def("get_seed",
		[]() { return static_cast<long long>(random_engine::get_seed()); },
		"Return the current seed of the global random engine.");

	// ---- Noise-parameter key type and architecture constants ----
	py::enum_<OperationType>(m, "OperationType", R"doc(
Enum of QRAM operation types.

Serves both as the key type of noise models (dict keys of
set_noise_models) and as the type tag of an Operation. Noise members
(BitFlip / PhaseFlip / BitPhaseFlip / Depolarizing / Damping, etc.) take
occurrence probabilities in [0, 1]; Damping additionally requires
gamma < 1 (gamma = 1 decays all excitations within a single step,
breaking sampling and normalization).
)doc")
		.value("ControlSwap", OperationType::ControlSwap)
		.value("HadamardData", OperationType::HadamardData)
		.value("CopyIn", OperationType::CopyIn)
		.value("CopyOut", OperationType::CopyOut)
		.value("SwapInternal", OperationType::SwapInternal)
		.value("FirstCopy", OperationType::FirstCopy)
		.value("FetchData", OperationType::FetchData)
		.value("SetZero", OperationType::SetZero)
		.value("Damping", OperationType::Damping)
		.value("Damp_Common", OperationType::Damp_Common)
		.value("Damp_Full", OperationType::Damp_Full)
		.value("BitFlip", OperationType::BitFlip)
		.value("PhaseFlip", OperationType::PhaseFlip)
		.value("BitPhaseFlip", OperationType::BitPhaseFlip)
		.value("Depolarizing", OperationType::Depolarizing)
		.value("Identity_Active", OperationType::Identity_Active)
		.value("Identity_Inactive", OperationType::Identity_Inactive)
		.export_values();

	m.attr("ARCH_QUBIT") = arch_qubit;
	m.attr("ARCH_QUTRIT") = arch_qutrit;

	// ---- Schedule data structures (lightweight read-only views) ----
	py::class_<Operation>(m, "Operation", "A single quantum operation: type, target qubits, and optional coefficients (noise parameters).")
		.def_readonly("type", &Operation::type, "Operation type (OperationType).")
		.def_readonly("targets", &Operation::targets, "List of target qubit / node indices.")
		.def_readonly("coefficients", &Operation::coefficients, "Noise coefficients (e.g. gamma for Damping).")
		.def_readonly("dagger", &Operation::dagger, "Whether this is the conjugate-transpose (inverse) operation.")
		.def("reverse", &Operation::reverse, "Return the inverse of this operation.")
		.def("to_string", &Operation::to_string, "Return a human-readable string representation.")
		.def("__str__", &Operation::to_string);

	py::class_<OperationPack>(m, "OperationPack", "A set of operations executed concurrently within one time slice (identified by name).")
		.def_readwrite("name", &OperationPack::name, "Name of this operation group.")
		.def_readonly("operations", &OperationPack::operations,
			"Operations of this slice, in execution order (list[Operation]; reads return a copy).")
		.def("reverse", &OperationPack::reverse, "Return the inverse operation group (each operation inverted, then reordered as in the original).")
		.def("empty", &OperationPack::empty, "Whether this operation group is empty.")
		.def("to_string", &OperationPack::to_string, "Return a human-readable string representation.")
		.def("__str__", &OperationPack::to_string);

	py::class_<TimeSlices>(m, "TimeSlices", "The complete QRAM loading schedule: a sequence of OperationPacks ordered by time slice.")
		.def_readwrite("time_slices", &TimeSlices::time_slices,
			"List of time slices (list[OperationPack]; reads return a copy).")
		.def("reverse", &TimeSlices::reverse, "Return the reversed schedule (overall order flipped and each group inverted).")
		.def("clear", &TimeSlices::clear, "Clear the schedule.")
		.def("to_string", &TimeSlices::to_string, "Return a human-readable string representation.")
		.def("__str__", &TimeSlices::to_string)
		.def("__len__", [](const TimeSlices& self) { return self.time_slices.size(); },
			"Number of time slices.");

	// ---- TimeStep: timing & noise scheduler ----
	py::class_<TimeStep>(m, "TimeStep", R"doc(
Timing and noise scheduler for the QRAM loading circuit.

Derives the routing, copy, and swap operations of each time slice from
the address / data widths, and can insert noise operations after every
time slice according to the noise models, producing the complete
TimeSlices schedule. QRAMCircuit holds a TimeStep instance internally
and calls generate in initialize_system to produce the noisy operation
sequence.
)doc")
		.def(py::init<size_t, size_t>(), py::arg("addr_size"), py::arg("data_size"),
			"Construct the scheduler.\n\nArgs:\n    addr_size: address width\n    data_size: data width")
		.def_readonly("addr_size", &TimeStep::addr_size, "Address width.")
		.def_readonly("data_size", &TimeStep::data_size, "Data width.")
		.def_readwrite("time_slices_noise_free", &TimeStep::time_slices_noise_free,
			"Noise-free schedule (TimeSlices).")
		.def("full_step", &TimeStep::full_step, "Total number of time slices of the full loading procedure.")
		.def("generate_step", &TimeStep::generate_step, py::arg("step"),
			"Generate the operation group of the given time slice (noise-free).\n\nArgs:\n    step: time-slice index (1-based)")
		.def("generate", &TimeStep::generate, py::arg("noises"), py::arg("arch_type"),
			"Generate the noisy schedule.\n\nArgs:\n"
			"    noises: noise models, a {OperationType: probability} dict (may be empty)\n"
			"    arch_type: architecture constant, ARCH_QUBIT or ARCH_QUTRIT")
		.def("is_bad_branch", &TimeStep::is_bad_branch, py::arg("addr"),
			"Check whether address addr falls inside the bad-branch range (bad range).")
		.def("layer_entangle_max", &TimeStep::layer_entangle_max, py::arg("step"),
			"Maximum number of tree layers entangled at the given time slice.\n\n"
			"Position-sampled noise of that slice lives on the tree positions\n"
			"[0, 2*(2^l - 1)); steps with l = 0 carry none (addr_size = 1 therefore\n"
			"has no position-sampled noise; under Damping only Damp_Common acts).")
		.def("get_bad_range_qubit", &TimeStep::get_bad_range_qubit, py::arg("bad_qubit"),
			"Qubit architecture: the bad-branch address range [lo, hi] derived from the bad qubit.")
		.def("get_bad_range_qutrit", &TimeStep::get_bad_range_qutrit, py::arg("bad_qubit"),
			"Qutrit architecture: the bad-branch address range [lo, hi] derived from the bad qubit.")
		.def("print", &TimeStep::print, "Print the detailed schedule of all time slices (fmt output to stdout).");

	// ---- Qubit-architecture QRAMCircuit ----
	{
		using QC = qram_qubit::QRAMCircuit;
		py::class_<QC>(m, "QRAMCircuitQubit", R"doc(
Branch-based sparse simulator of the qubit-architecture QRAM loading circuit.

Carries (address, bus_input) input branches as trajectories, evolves the
sparse SystemState along the noisy operation sequence generated by
TimeStep, and supports good/bad branch pruning (run_normal), the
unpruned reference (run_full), and fidelity sampling.

Typical workflow:
    qram = QRAMCircuitQubit(addr_size, data_size[, memory])
    qram.set_noise_models({...})       # optional
    qram.set_input_uniform(n)          # or set_input_random(n)
    qram.run_normal()                  # or run_full() / run("normal")
    qram.sample_and_get_fidelity()
)doc")
			.def(py::init<size_t, size_t>(), py::arg("addr_size"), py::arg("data_size"),
				"Construct the QRAM circuit and allocate 2^addr_size memory slots (uninitialized).")
			.def(py::init([](size_t a, size_t d, const memory_t& mem) { return QC(a, d, memory_t(mem)); }),
				py::arg("addr_size"), py::arg("data_size"), py::arg("memory"),
				"Construct the QRAM circuit initialized with the given memory (memory length must be 2^addr_size).")
			.def_readonly("addr_size", &QC::addr_size, "Address width.")
			.def_readonly("data_size", &QC::data_size, "Data width.")
			.def("set_memory_random", &QC::set_memory_random,
				"Randomly fill the entire data tree using the global random engine.")
			.def("set_memory", (void (QC::*)(const memory_t&)) & QC::set_memory,
				py::arg("memory"), "Set the data tree (memory length must be 2^addr_size).")
			.def("get_memory", [](const QC& self) -> const memory_t& { return self.get_memory(); },
				py::return_value_policy::reference_internal, "Return the data tree (reference view).")
			.def("get_qubit_num", &QC::get_qubit_num,
				"Return the number of physical qubits used by the circuit: 2 * (2^addr_size - 1).")
			.def("memory_size", &QC::memory_size, "Return the total number of memory slots.")
			.def("set_input_random", &QC::set_input_random, py::arg("n_inputs"),
				"Randomly sample n input branches, drawing input probabilities ~ U(0,1) then normalizing.")
			.def("set_input_uniform", &QC::set_input_uniform, py::arg("n_inputs"),
				"Randomly sample n input branches with equal input probability each.")
			.def("set_input_zerobus", [](QC& self) {
				/* zerobus input of the correspondence experiments: uniform
				   superposition over all addresses with bus = 0 (a uniform bus
				   makes the joint output distribution degenerate, losing
				   discriminative power) */
				self.branch_groups.clear();
				size_t naddr = pow2(self.addr_size);
				for (size_t a = 0; a < naddr; ++a)
				{
					self.branch_groups.emplace_back(a);
					auto& group = self.branch_groups.back();
					group.branches_input.emplace_back(a, self.data_size, 0);
					group.branch_probs.push_back(1.0 / (double)naddr);
					group.state_probs.push_back(1.0 / (double)naddr);
				}
			},
				"Set the zerobus input: an equal-probability branch per address, all with\n"
				"bus input 0 (deterministic, no RNG draw; the discriminating input used by\n"
				"the circuit-level baseline comparison).")
			.def("set_noise_models", &QC::set_noise_models, py::arg("noises"),
				"Set noise models {OperationType: probability}.\n\n"
				"Probabilities must lie in [0,1]; Damping requires gamma < 1, otherwise a RuntimeError is raised.")
			.def("get_noise_models", [](const QC& self) -> const noise_t& { return self.noise_parameters; },
				py::return_value_policy::reference_internal, "Return the current noise models (reference view).")
			.def("is_noise_free", &QC::is_noise_free, "Whether no noise has been set.")
			.def("has_damping", &QC::has_damping, "Whether the current noise models include Damping.")
			.def("initialize_system", &QC::initialize_system,
				"Generate the noisy operation sequence via TimeStep and reset all branch groups (called internally by the run family).")
			.def("run_normal", &QC::run_normal,
				"Pruned run: all bad branches plus one good branch as reference (the default entry point for daily use).")
			.def("run_full", &QC::run_full,
				"Unpruned run over all branches, serving as the ground-truth reference of the pruned result.")
			.def("run", (void (QC::*)()) & QC::run,
				"Generate the operation sequence and run in pruned mode (the internal entry equivalent to run_normal).")
			.def("run", (void (QC::*)(std::string)) & QC::run, py::arg("version"),
				"Run by version string: \"full\" (unpruned) / \"normal\" (pruned) / \"fast\" (good branches only).")
			.def("sample_and_get_fidelity", &QC::sample_and_get_fidelity,
				"Sample the output branches and compute the loading fidelity (squared modulus of the inner product between the ideal and actual states).")
			.def("get_fidelity", &QC::get_fidelity,
				"Alias of sample_and_get_fidelity.")
			.def("get_output_distribution", [](const QC& self) {
				/* marginal (address:bus) output distribution over all branch
				   groups -- the quantity the circuit-level baseline compares
				   against; sub-normalized under Damping (sum = survival) */
				std::map<std::string, double> dist;
				for (auto& group : self.get_branch_groups())
					for (size_t b = 0; b < group.branches.size(); ++b)
						for (auto it = group.branches[b].iterbeg();
							it != group.branches[b].iterend(); ++it)
							dist[fmt::format("{}:{}", group.address, it->data_bus)]
								+= group.branch_probs[b] * abs_sqr(it->amplitude);
				return dist;
			},
				"Return the marginal (address:bus) output distribution of the current\n"
				"branch groups as a {\"addr:bus\": prob} dict (call after run_full / run_normal;\n"
				"mutating calls such as sample_and_get_fidelity invalidate it).")
			.def("get_fidelity_conventions", [](const QC& self) {
				/* per-trajectory fidelity statistics without output sampling --
				   the three no-post-selection conventions of the correspondence
				   experiments (computed on the current branch groups) */
				bus_t bus_mask = pow2(self.data_size) - 1;
				complex_t coh = 0, ov = 0;
				double incoh = 0;
				for (auto& group : self.get_branch_groups())
					for (size_t b = 0; b < group.branches.size(); ++b)
					{
						auto& branch = group.branches[b];
						auto expect_bus = (branch.bus_input ^ self.memory[group.address]) & bus_mask;
						complex_t Fa = 0, Ga = 0;
						for (auto it = branch.iterbeg(); it != branch.iterend(); ++it)
						{
							if (it->data_bus == expect_bus)
							{
								Fa += it->amplitude;
								if (it->state.nz_elements.empty())
									Ga += it->amplitude;
							}
						}
						coh += group.branch_probs[b] * Fa;
						ov += group.branch_probs[b] * Ga;
						incoh += group.branch_probs[b] * abs_sqr(Fa);
					}
				py::dict ret;
				ret["fid_nopost"] = abs_sqr(coh);  // tree-insensitive: any tree state counts
				ret["overlap_fid"] = abs_sqr(ov);  // tree-sensitive = |<psi_ideal|psi_traj>|^2
				ret["fid_incoh"] = incoh;          // branch-projection (incoherent) combination
				return ret;
			},
				"Return the three no-post-selection fidelity statistics of the current\n"
				"run as a dict (call after run_full, before sample_and_get_fidelity):\n\n"
				"  overlap_fid: |<psi_ideal|psi_traj>|^2, tree-sensitive -- statistically\n"
				"    matches the channel-level <psi_ideal|rho|psi_ideal> of the baseline;\n"
				"  fid_nopost: same inner product but tree-insensitive (bus-correct\n"
				"    amplitudes with residual tree excitations still counted);\n"
				"  fid_incoh: incoherent (branch-projected) combination of the same.")
			.def("to_string", &QC::to_string, "Return a summary of the circuit.")
			.def("to_string_full_info", &QC::to_string_full_info,
				"Return the full information including branch details.")
			.def("__str__", &QC::to_string);
	}

	// ---- Qutrit-architecture QRAMCircuit ----
	{
		using QC = qram_qutrit::QRAMCircuit;
		// module_local: this C++ type is also registered by the rich bindings
		// of the pysparq package (Python name QRAMCircuit_qutrit); pybind11
		// registers by C++ typeid globally, so importing both modules would
		// raise "already registered" for the later import. With module_local
		// the two packages coexist; instances of this type are only used
		// inside the qram_simulator module and never flow across modules.
		py::class_<QC>(m, "QRAMCircuitQutrit", py::module_local(), R"doc(
Branch-based sparse simulator of the qutrit-architecture QRAM loading circuit.

Each tree node is a three-level system (addr ∈ {W, L, R}, data), evolved
along the noisy operation sequence via SubBranch trajectories. The
workflow matches QRAMCircuitQubit: construct -> set memory -> set noise
-> set_input_* -> run_normal/run_full -> sample_and_get_fidelity.
)doc")
			.def(py::init<size_t, size_t>(), py::arg("addr_size"), py::arg("data_size"),
				"Construct the QRAM circuit and allocate 2^addr_size memory slots (uninitialized).")
			.def(py::init<size_t, size_t, const memory_t&>(),
				py::arg("addr_size"), py::arg("data_size"), py::arg("memory"),
				"Construct the QRAM circuit initialized with the given memory.")
			.def_readonly("address_size", &QC::address_size, "Address width.")
			.def_readonly("data_size", &QC::data_size, "Data width.")
			.def("set_memory_random", &QC::set_memory_random,
				"Randomly fill the entire data tree using the global random engine.")
			.def("set_memory", (void (QC::*)(const memory_t&)) & QC::set_memory,
				py::arg("memory"), "Set the data tree.")
			.def("get_memory", [](const QC& self) -> const memory_t& { return self.get_memory(); },
				py::return_value_policy::reference_internal, "Return the data tree (reference view).")
			.def("get_qubit_num", &QC::get_qubit_num,
				"Return the number of physical qubits used by the circuit: 2 * (2^address_size - 1).")
			.def("memory_size", &QC::memory_size, "Return the total number of memory slots.")
			.def("set_input_random", &QC::set_input_random, py::arg("n_inputs"),
				"Randomly sample n input branches and fill the branch probabilities.")
			.def("set_input_uniform", &QC::set_input_uniform, py::arg("n_inputs"),
				"Randomly sample n input branches with equal input probability each.")
			.def("set_noise_models", &QC::set_noise_models, py::arg("noises"),
				"Set noise models {OperationType: probability}.\n\n"
				"Probabilities must lie in [0,1]; Damping requires gamma < 1, otherwise a RuntimeError is raised.")
			.def("get_noise_models", [](const QC& self) -> const noise_t& { return self.noise_parameters; },
				py::return_value_policy::reference_internal, "Return the current noise models (reference view).")
			.def("is_noise_free", &QC::is_noise_free, "Whether no noise has been set.")
			.def("has_damping", &QC::has_damping, "Whether the current noise models include Damping.")
			.def("initialize_system", &QC::initialize_system,
				"Generate the noisy operation sequence via TimeStep and initialize the branch structure (called internally by the run family).")
			.def("run_normal", &QC::run_normal,
				"Pruned run: all bad branches plus one good branch as reference (the default entry point for daily use).")
			.def("run_full", &QC::run_full,
				"Unpruned run over all branches, serving as the ground-truth reference of the pruned result.")
			.def("run", (void (QC::*)(const std::string&)) & QC::run, py::arg("version"),
				"Run by version string (\"full\" / \"normal\", etc.; same as QRAMCircuitQubit.run).")
			.def("sample_and_get_fidelity", &QC::sample_and_get_fidelity,
				"Sample the output branches and compute the loading fidelity.")
			.def("get_branch_probs",
				[](const QC& self) -> const std::vector<double>& { return self.get_branch_probs(); },
				py::return_value_policy::reference_internal,
				"Return the normalized probabilities of all branches (reference view).")
			.def("to_string", &QC::to_string, "Return a summary of the circuit.")
			.def("to_string_full_info", &QC::to_string_full_info,
				"Return the full information including branch details.")
			.def("__str__", &QC::to_string);
	}

	// ---- QRAMFullAmp: full-amplitude state-vector bridge ----
	{
		py::class_<QRAMFullAmp>(m, "QRAMFullAmp", R"doc(
Bridge from the QRAM circuit to a full-amplitude state vector (qutrit
architecture only).

An external full-amplitude simulator (or a user script) supplies the
state vector and qubit mapping; apply composites one QRAM loading onto
that state vector and returns the new vector, embedding QRAM simulation
into a larger circuit-evolution pipeline.

Example:

    manipulator = QRAMFullAmp(2, 2, [0, 1, 2, 3])
    manipulator.set_noise_models({OperationType.Depolarizing: 1e-3})
    out = manipulator.apply(state, [0, 1], [2, 3], [], version="normal")
)doc")
			.def(py::init<size_t, size_t, const memory_t&>(),
				py::arg("addr_size"), py::arg("data_size"), py::arg("memory"),
				"Construct the bridge: internally creates a qutrit QRAMCircuit loaded with the given memory.")
			.def("set_noise_models",
				[](QRAMFullAmp& self, const noise_t& noises) { self->set_noise_models(noises); },
				py::arg("noises"),
				"Set the noise models of the internal QRAMCircuit (same constraints as QRAMCircuitQutrit).")
			.def("apply", &QRAMFullAmp::apply,
				py::arg("state"), py::arg("address_qubits"), py::arg("data_qubits"),
				py::arg("other_qubits"), py::arg("version") = "normal",
				"Composite one QRAM loading onto a full-amplitude state vector.\n\n"
				"Args:\n"
				"    state: list of complex amplitudes (length 2^total qubit count)\n"
				"    address_qubits / data_qubits / other_qubits: indices of the three qubit groups\n"
				"    version: \"normal\" (pruned) or \"full\" (unpruned)\n"
				"Returns:\n"
				"    the new state vector after loading (list of complex numbers)");
	}
}
