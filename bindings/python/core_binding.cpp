#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-value"
#endif

#include "pybind11/pybind11.h"
#include "pybind11/stl.h"
#include "pybind11/complex.h"

#include "sparse_state_simulator.h"

using namespace pybind11::literals;
namespace py = pybind11;
using namespace qram_simulator;

using namespace std;

// Minimal binding helpers (the full-featured macros, including the
// conditioned_by_* control surface, live in SparQSim's pysparq bindings).
#define BIND_BASE_OPERATOR(NAME) py::class_<NAME, BaseOperator>(m, #NAME)

/* Workaround for binding inherited operator() of SelfAdjointOperator
 *
   This is used to solve the issue for Windows platform which has a trouble if
   calling operator() in Python. However, the underlying mechanism is unclear,
   so I just use a workaround to avoid the issue. */
#ifdef _WIN32
#define BIND_SELF_ADJOINT_OPERATOR(NAME)                                                                            \
    py::class_<NAME, SelfAdjointOperator>(m, #NAME)                                                                 \
        .def("__call__", (void (NAME::*)(SparseState &) const) & NAME::operator(), py::arg("state"))
#else
#define BIND_SELF_ADJOINT_OPERATOR(NAME) py::class_<NAME, SelfAdjointOperator>(m, #NAME)
#endif

#define BIND_DAG_METHODS(NAME) \
    .def("dag", (void (NAME::*)(SparseState &) const) & NAME::dag, py::arg("state"))

PYBIND11_MODULE(_core, m)
{
    m.doc() = R"doc(
qram_simulator - Thin Python bindings for the qram-simulator C++ core.

Exposes the core sparse-state simulator primitives: register management
(System), SparseState, basic arithmetic/gate operators, measurement, and
native QRAM loading. This is a deliberately minimal surface; the
full-featured Register Level Programming API (algorithms, RIR interpreter,
dynamic operators, operator conditioning) is published separately as the
`pysparq` package from the SparQSim repository.

Example:
    from qram_simulator import System, SparseState, StateStorageType, \
        Init_Unsafe, Hadamard_Int

    System.add_register("q", StateStorageType.UnsignedInteger, 4)
    state = SparseState()
    Init_Unsafe("q", 5)(state)
    Hadamard_Int("q", 4)(state)
    print(state)
)doc";

    py::enum_<StateStorageType>(m, "StateStorageType")
        .value("General", StateStorageType::General)
        .value("UnsignedInteger", StateStorageType::UnsignedInteger)
        .value("SignedInteger", StateStorageType::SignedInteger)
        .value("Boolean", StateStorageType::Boolean)
        .value("Rational", StateStorageType::Rational)
        .export_values();

    py::class_<StateStorage>(m, "StateStorage")
        .def(py::init<>())
        .def_readonly("value", &StateStorage::value);

    py::class_<SparseState> sparse_state(m, "SparseState", R"doc(
Sparse quantum state representation.

Stores only non-zero amplitude entries, making it efficient for states
with limited superposition. Works with the global System registry.
)doc");
    sparse_state.def(py::init<>(), "Create an empty sparse quantum state")
        .def(py::init<const SparseState &>(), py::arg("other"),
             "Copy amplitude/register values from another sparse state. "
             "The System register namespace remains process-global.")
        .def("clone", [](const SparseState& self) { return SparseState(self); },
             "Copy amplitude/register values. The System register namespace "
             "remains process-global.")
        .def_readonly("basis_states", &SparseState::basis_states)
        .def("size", &SparseState::size)
        .def("empty", &SparseState::empty)
        .def("to_string", &SparseState::to_string, py::arg("display") = 0,
             py::arg("precision") = 0,
             "Return a formatted string representation of the state.\n\n"
             "Args:\n"
             "    display: Display mode flags (StatePrintDisplay values).\n"
             "    precision: Number of decimal places for floating-point numbers.")
        .def("__str__",  [](const SparseState& self) { return self.to_string(1); })
        .def("__repr__", [](const SparseState& self) { return self.to_string(1); });

    py::class_<System>(m, "System", R"doc(
Quantum system managing named registers.

The System class provides the foundation for register management. It tracks
register names, types, and sizes via a global registry shared by all
SparseState instances.
)doc")
        .def(py::init<>(), "Create an empty quantum system")
        .def_readonly("amplitude", &System::amplitude)
        .def_readonly("registers", &System::registers)
        .def_readonly_static("name_register_map", &System::name_register_map)
        .def_readonly_static("max_register_count", &System::max_register_count)
        .def("last_register", (StateStorage & (System::*)()) & System::last_register,
             py::return_value_policy::reference_internal)
        .def_static("clear", &System::clear)
        .def_static("get_qubit_count", &System::get_qubit_count)
        .def_static("get_activated_register_size", &System::get_activated_register_size)
        .def_static("get_id", (size_t (*)(std::string_view))&System::get, py::arg("name"))
        .def_static("name_of", &System::name_of, py::arg("id"))
        .def_static("type_of", (StateStorageType (*)(std::string_view))&System::type_of, py::arg("name"))
        .def_static("type_of", (StateStorageType (*)(size_t))&System::type_of, py::arg("id"))
        .def_static("size_of", (size_t (*)(std::string_view))&System::size_of, py::arg("name"))
        .def_static("size_of", (size_t (*)(size_t))&System::size_of, py::arg("id"))
        .def_static("status_of", (bool (*)(std::string_view))&System::status_of, py::arg("name"))
        .def_static("status_of", (bool (*)(size_t))&System::status_of, py::arg("id"))
        .def_static("add_register", &System::add_register, py::arg("name"),
             py::arg("type"), py::arg("size"))
        .def_static("add_register_synchronous",
             (size_t (*)(std::string_view, StateStorageType, size_t, SparseState &))&System::add_register_synchronous,
             py::arg("name"), py::arg("type"), py::arg("size"), py::arg("state"))
        .def_static("remove_register", (void (*)(size_t))&System::remove_register, py::arg("id"))
        .def_static("remove_register", (void (*)(std::string_view))&System::remove_register, py::arg("name"))
        .def_static("remove_register_synchronous",
             (void (*)(size_t, SparseState &))&System::remove_register_synchronous,
             py::arg("id"), py::arg("state"))
        .def_static("remove_register_synchronous",
             (void (*)(std::string_view, SparseState &))&System::remove_register_synchronous,
             py::arg("name"), py::arg("state"))
        .def("__str__", (std::string (System::*)() const) & System::to_string)
        .def("to_string", (std::string (System::*)() const) & System::to_string);

    py::class_<BaseOperator>(m, "BaseOperator")
        .def("__call__", (void (BaseOperator::*)(SparseState &) const) & BaseOperator::operator())
        .def("dag", (void (BaseOperator::*)(SparseState &) const) & BaseOperator::dag);

    BIND_BASE_OPERATOR(SelfAdjointOperator)
#ifdef _WIN32
        .def("__call__", (void (SelfAdjointOperator::*)(SparseState &) const) & SelfAdjointOperator::operator())
#endif
        .def("dag", (void (SelfAdjointOperator::*)(SparseState &) const) & SelfAdjointOperator::dag);

    // ---- dark_magic.h / debugger.h ----
    BIND_SELF_ADJOINT_OPERATOR(Normalize)
        .def(py::init<>());

    BIND_SELF_ADJOINT_OPERATOR(Init_Unsafe, R"doc(
Initialize a register to a specific value (unsafe).

Sets the register to a classical value without checking normalization.

Args:
    reg: Register name (str) or ID (int).
    value: Classical value to set.

Example:
    Init_Unsafe("q", 5)(state)
)doc")
        .def(py::init<std::string_view, size_t>(), py::arg("reg"), py::arg("value"))
        .def(py::init<size_t, size_t>(), py::arg("id"), py::arg("value"));

    BIND_SELF_ADJOINT_OPERATOR(CheckNormalization)
        .def(py::init<>())
        .def(py::init<double>(), py::arg("threshold"));

    BIND_SELF_ADJOINT_OPERATOR(ClearZero)
        .def(py::init<>())
        .def(py::init<double>(), py::arg("epsilon"));

    // ---- State printing ----
    py::enum_<StatePrintDisplay>(m, "StatePrintDisplay")
        .value("Default", StatePrintDisplay::Default)
        .value("Detail", StatePrintDisplay::Detail)
        .value("Binary", StatePrintDisplay::Binary)
        .value("Prob", StatePrintDisplay::Prob)
        .export_values();

    // StatePrint — declared as a plain class_ (NOT via the macro): the macro's
    // void(SparseState&) __call__ overload would shadow the py::object overload
    // below on exact-type matches and silently return None.
    py::class_<StatePrint, SelfAdjointOperator>(m, "StatePrint")
        .def(py::init<int32_t>(), py::arg("disp") = 0)
        .def(py::init<int32_t, int>(), py::arg("disp"), py::arg("precision"))
        .def(py::init<StatePrintDisplay>(), py::arg("disp"))
        .def_readwrite_static("on", &StatePrint::on)
        .def("__call__", [](StatePrint& self, py::object state_obj) -> std::string {
            py::list sys_list = state_obj.attr("basis_states").cast<py::list>();
            std::vector<System> systems;
            systems.reserve(py::len(sys_list));
            for (py::handle item : sys_list) {
                System& sys = item.cast<System&>();
                systems.push_back(sys);
            }
            return self.to_string(systems);
        }, py::arg("state"),
            "Return formatted state string for the given SparseState.");

    m.def("print", [](SparseState& state) {
        std::string result = state.to_string(1);  // Detail mode
        py::print(result, "end"_a = "");
    }, py::arg("state"),
        "Print a SparseState to stdout in detail mode.");

    // ---- hadamard.h ----
    BIND_SELF_ADJOINT_OPERATOR(Hadamard_Int, R"doc(
Apply Hadamard transform to an integer register.

Creates an equal superposition over all integer values from 0 to 2^n - 1
for the specified number of digits.

Example:
    Hadamard_Int("q", 4)(state)  # Superpose q over 0..15
)doc")
        .def(py::init<std::string_view, size_t>(), py::arg("reg_in"), py::arg("n_digits"))
        .def(py::init<size_t, size_t>(), py::arg("reg_in"), py::arg("n_digits"));

    BIND_SELF_ADJOINT_OPERATOR(Hadamard_Int_Full)
        .def(py::init<std::string_view>(), py::arg("reg_in"))
        .def(py::init<size_t>(), py::arg("reg_in"));

    BIND_SELF_ADJOINT_OPERATOR(Hadamard_Bool)
        .def(py::init<std::string_view>(), py::arg("reg_in"))
        .def(py::init<size_t>(), py::arg("reg_in"));

    // ---- basic_gates.h ----
    BIND_SELF_ADJOINT_OPERATOR(X_Bool)
        .def(py::init<std::string_view, size_t>(), py::arg("reg"), py::arg("digit"))
        .def(py::init<size_t, size_t>(), py::arg("reg_id"), py::arg("digit"));

    BIND_SELF_ADJOINT_OPERATOR(FlipBools)
        .def(py::init<std::string_view>(), py::arg("reg"))
        .def(py::init<size_t>(), py::arg("reg_id"));

    // ---- quantum_arithmetic.h ----
    BIND_SELF_ADJOINT_OPERATOR(Add_UInt_UInt, R"doc(
Add two unsigned integer registers.

Computes: |a>|b>|0> -> |a>|b>|a+b> (mod 2^n)

Example:
    Add_UInt_UInt("a", "b", "result")(state)  # result = a + b
)doc")
        .def(py::init<std::string_view, std::string_view, std::string_view>(),
             py::arg("input_reg1"), py::arg("input_reg2"), py::arg("output_reg"))
        .def(py::init<size_t, size_t, size_t>(),
             py::arg("input_id1"), py::arg("input_id2"), py::arg("output_id"));

    BIND_BASE_OPERATOR(Add_UInt_UInt_InPlace)
        .def(py::init<std::string_view, std::string_view>(),
             py::arg("input_reg"), py::arg("output_reg"))
        .def(py::init<size_t, size_t>(),
             py::arg("input_id"), py::arg("output_id"))
            BIND_DAG_METHODS(Add_UInt_UInt_InPlace);

    BIND_SELF_ADJOINT_OPERATOR(Add_UInt_ConstUInt)
        .def(py::init<std::string_view, size_t, std::string_view>(),
             py::arg("input_reg"), py::arg("add"), py::arg("output_reg"))
        .def(py::init<size_t, size_t, size_t>(),
             py::arg("input_id"), py::arg("add"), py::arg("output_id"));

    BIND_BASE_OPERATOR(Add_ConstUInt_InPlace)
        .def(py::init<std::string_view, size_t>(), py::arg("input_reg"), py::arg("add"))
        .def(py::init<size_t, size_t>(), py::arg("input_id"), py::arg("add"))
            BIND_DAG_METHODS(Add_ConstUInt_InPlace);

    // ---- qft.h ----
    BIND_BASE_OPERATOR(QFT)
        .def(py::init<std::string_view>(), py::arg("reg_name"))
        .def(py::init<size_t>(), py::arg("reg_id"))
            BIND_DAG_METHODS(QFT);

    BIND_BASE_OPERATOR(InverseQFT)
        .def(py::init<std::string_view>(), py::arg("reg_name"))
        .def(py::init<size_t>(), py::arg("reg_id"));

    // ---- random_engine.h ----
    m.def("set_seed", [](long long seed) { random_engine::set_seed(static_cast<seed_t>(seed)); },
          py::arg("seed"),
          "Seed the global random engine used by measurement/reset/PartialTrace.");

    m.def("get_seed", []() { return static_cast<long long>(random_engine::get_seed()); },
          "Return the current seed of the global random engine.");

    // ---- measurement.h ----
    py::class_<MeasureZ>(m, "MeasureZ", R"doc(
Projective Z-basis (computational basis) measurement.

Samples an outcome for one or more registers according to the Born rule,
using the seedable global random engine (see set_seed()). Collapses the
state onto the sampled branch and renormalizes it in place.

Example:
    set_seed(0)
    outcome, prob = MeasureZ("q")(state)
)doc")
        .def(py::init<const std::vector<std::string> &>(), py::arg("register_names"))
        .def(py::init<const std::vector<size_t> &>(), py::arg("register_ids"))
        .def(py::init<std::string_view>(), py::arg("register_name"))
        .def(py::init<size_t>(), py::arg("register_id"))
        .def_readonly("registers", &MeasureZ::registers)
        .def("__call__",
             (std::pair<std::vector<uint64_t>, double> (MeasureZ::*)(SparseState &) const) & MeasureZ::operator(),
             py::arg("state"));

    py::class_<Reset>(m, "Reset", R"doc(
Reset one or more registers to a definite classical value (default 0).

Implemented as measurement (collapse + renormalize) followed by a
deterministic classical correction. Returns the pre-reset measured outcome.

Example:
    set_seed(0)
    measured = Reset("q", 3)(state)  # reset "q" to 3
)doc")
        .def(py::init<std::string_view, uint64_t>(), py::arg("register_name"), py::arg("target") = 0)
        .def(py::init<size_t, uint64_t>(), py::arg("register_id"), py::arg("target") = 0)
        .def_readonly("registers", &Reset::registers)
        .def_readonly("target_values", &Reset::target_values)
        .def("__call__",
             (std::vector<uint64_t> (Reset::*)(SparseState &) const) & Reset::operator(), py::arg("state"));

    py::class_<Probability>(m, "Probability", R"doc(
Read-only Born-rule probability query (does not modify the state).

Example:
    p = Probability("q", 5)(state)
    dist = Probability.distribution(state, "q")  # full outcome distribution
)doc")
        .def(py::init<std::string_view, uint64_t>(), py::arg("register_name"), py::arg("value"))
        .def(py::init<size_t, uint64_t>(), py::arg("register_id"), py::arg("value"))
        .def("__call__",
             (double (Probability::*)(const SparseState &) const) & Probability::operator(), py::arg("state"))
        .def_static("distribution",
             (std::map<uint64_t, double> (*)(const SparseState &, size_t)) & Probability::distribution,
             py::arg("state"), py::arg("register_id"))
        .def_static("distribution",
             (std::map<uint64_t, double> (*)(const SparseState &, std::string_view)) & Probability::distribution,
             py::arg("state"), py::arg("register_name"));

    // ---- partial_trace.h ----
    py::class_<PartialTrace>(m, "PartialTrace")
        .def(py::init<const std::vector<std::string> &>(), py::arg("partial_trace_register_names"))
        .def(py::init<const std::vector<size_t> &>(), py::arg("partial_trace_register_ids"))
        .def(py::init<std::string>(), py::arg("single_register_name"))
        .def(py::init<size_t>(), py::arg("single_register_id"))
        .def("__call__",
             (std::pair<std::vector<uint64_t>, double> (PartialTrace::*)(SparseState &) const) & PartialTrace::operator(),
             py::arg("state"));

    py::class_<PartialTraceSelect>(m, "PartialTraceSelect")
        .def(py::init<const std::map<std::string_view, uint64_t> &>(), py::arg("name_value_map"))
        .def(py::init<const std::map<size_t, uint64_t> &>(), py::arg("id_value_map"))
        .def("__call__",
             (double (PartialTraceSelect::*)(SparseState &) const) & PartialTraceSelect::operator(),
             py::arg("state"));

    // ---- qram.h ----
    py::class_<qram_qutrit::QRAMCircuit>(m, "QRAMCircuit_qutrit")
        .def(py::init<size_t, size_t>(), py::arg("addr_size"), py::arg("data_size"))
        .def(py::init<size_t, size_t, const memory_t &>(), py::arg("addr_size"), py::arg("data_size"), py::arg("memory"))
        .def(py::init<size_t, size_t, memory_t &&>(), py::arg("addr_size"), py::arg("data_size"), py::arg("memory"))
        .def_readonly("address_size", &qram_qutrit::QRAMCircuit::address_size)
        .def_readonly("data_size", &qram_qutrit::QRAMCircuit::data_size);

    BIND_SELF_ADJOINT_OPERATOR(QRAMLoad, R"doc(
Load classical data into quantum superposition via QRAM.

Args:
    qram: QRAMCircuit_qutrit instance containing the memory.
    addr_reg: Name/ID of the address register.
    data_reg: Name/ID of the data register.

Example:
    qram = QRAMCircuit_qutrit(addr_size=3, data_size=4, memory=data)
    QRAMLoad(qram, "address", "data")(state)
)doc")
        .def(py::init<qram_qutrit::QRAMCircuit *, std::string_view, std::string_view>(),
             py::arg("qram"), py::arg("addr_reg"), py::arg("data_reg"))
        .def(py::init<qram_qutrit::QRAMCircuit *, size_t, size_t>(),
             py::arg("qram"), py::arg("addr_reg_id"), py::arg("data_reg_id"))
        .def_readonly("qram_circuit", &QRAMLoad::qram)
        .def_readonly_static("version", &QRAMLoad::version);

    BIND_SELF_ADJOINT_OPERATOR(QRAMLoadFast)
        .def(py::init<qram_qutrit::QRAMCircuit *, std::string_view, std::string_view>(),
             py::arg("qram"), py::arg("addr_reg"), py::arg("data_reg"))
        .def(py::init<qram_qutrit::QRAMCircuit *, size_t, size_t>(),
             py::arg("qram"), py::arg("addr_reg_id"), py::arg("data_reg_id"));
}
