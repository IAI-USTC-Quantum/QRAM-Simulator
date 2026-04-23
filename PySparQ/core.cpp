#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-value"
#endif

#include "BindUtils.h"
#include "BlockEncoding/block_encoding_tridiagonal.h"
#include "hamiltonian_simulation.h"

PYBIND11_MODULE(_core, m)
{
    m.doc() = R"doc(
PySparQ - Sparse-state quantum circuit simulator with native QRAM support.

This module provides a Register Level Programming paradigm for quantum algorithm
development. Instead of composing circuits from individual gates, operate directly
on named registers using high-level arithmetic operations.

Key classes:
    System: Quantum system managing registers
    SparseState: Sparse quantum state representation
    BaseOperator: Base class for all quantum operators

Example:
    from pysparq import System, SparseState, AddRegister, Hadamard_Int

    system = System()
    state = SparseState()
    AddRegister("q", UnsignedInteger, 4)(state)
    Hadamard_Int("q")(state)
    print(state)
)doc";

    py::class_<DenseMatrix<complex_t>>(m, "DenseMatrix_complex")
        .def(py::init<>())
        .def(py::init<size_t>(), py::arg("size"));

    py::class_<DenseMatrix<double>>(m, "DenseMatrix_float64")
        .def(py::init<>())
        .def(py::init<size_t>(), py::arg("size"));

    py::class_<SparseMatrix>(m, "SparseMatrix")
        .def(py::init<>())
        .def(py::init<const std::vector<double> &, const std::vector<size_t> &, size_t, size_t, size_t, bool>());

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

Example:
    state = SparseState()
    AddRegister("q", UnsignedInteger, 4)(state)
    Hadamard_Int("q")(state)

Note:
    The sparse representation is memory-efficient but may be slower
    for dense superposition states.
)doc");
    sparse_state.def(py::init<>(), "Create an empty sparse quantum state");

    py::class_<System>(m, "System", R"doc(
Quantum system managing named registers.

The System class provides the foundation for register management. It tracks
register names, types, and sizes via a global registry shared by all
SparseState instances.

Example:
    system = System()
    state = SparseState()

Attributes:
    registers: Dict mapping register names to their metadata.
    amplitude: Amplitude coefficient for this system instance.
)doc")
        .def(py::init<>(), "Create an empty quantum system")
        .def_readonly("amplitude", &System::amplitude)
        .def_readonly("registers", &System::registers)
        .def_readonly_static("name_register_map", &System::name_register_map)
        .def_readonly_static("max_register_map", &System::max_qubit_count)
        .def_readonly_static("max_register_count", &System::max_register_count)
        .def_readonly_static("max_system_size", &System::max_system_size)
        .def_readonly_static("temporal_registers", &System::temporal_registers)
        .def_readonly_static("reusable_registers", &System::reusable_registers)
        .def("get", (StateStorage & (System::*)(size_t)) & System::get)
        .def("get", (const StateStorage &(System::*)(size_t) const) & System::get)
        .def_static("clear", &System::clear)
        .def_static("get_qubit_count", &System::get_qubit_count)
        .def_static("get_activated_register_size", &System::get_activated_register_size)
        .def("last_register", (StateStorage & (System::*)()) & System::last_register)
        .def("last_register", (const StateStorage &(System::*)() const) & System::last_register)
        .def_static("update_max_size", &System::update_max_size)
        .def_static("get_id", (size_t (*)(std::string_view))&System::get)
        .def_static("get_register_info", &System::get_register_info)
        .def_static("name_of", &System::name_of)
        .def_static("type_of", (StateStorageType (*)(std::string_view))&System::type_of)
        .def_static("type_of", (StateStorageType (*)(size_t))&System::type_of)
        .def_static("size_of", (size_t (*)(std::string_view))&System::size_of)
        .def_static("size_of", (size_t (*)(size_t))&System::size_of)
        .def_static("status_of", (bool (*)(std::string_view))&System::status_of)
        .def_static("status_of", (bool (*)(size_t))&System::status_of)
        .def_static("set_register_type", [](size_t reg_id, StateStorageType type) {
            if (reg_id >= System::name_register_map.size())
                throw py::value_error("Invalid register ID");
            std::get<1>(System::name_register_map[reg_id]) = type;
        }, py::arg("reg_id"), py::arg("type"))
        .def_static("add_register", &System::add_register)
        .def_static("add_register_synchronous", (size_t (*)(std::string_view, StateStorageType, size_t, SparseState &))&System::add_register_synchronous)
        .def_static("add_register_synchronous", (size_t (*)(std::string_view, StateStorageType, size_t, std::vector<System> &))&System::add_register_synchronous)
        .def_static("remove_register", (void (*)(size_t))&System::remove_register)
        .def_static("remove_register", (void (*)(std::string_view))&System::remove_register)
        .def_static("remove_register_synchronous", (void (*)(size_t, std::vector<System> &))&System::remove_register_synchronous)
        .def_static("remove_register_synchronous", (void (*)(std::string_view, std::vector<System> &))&System::remove_register_synchronous)
        .def("__less__", &System::operator<)
        .def("__eq__", &System::operator==)
        .def("__ne__", &System::operator!=)
        .def("__str__", (std::string (System::*)() const) & System::to_string)
        .def("to_string", (std::string (System::*)() const) & System::to_string)
        .def("to_string", (std::string (System::*)(int precision) const) & System::to_string);

    m.def("merge_system", &merge_system);
    m.def("remove_system", &remove_system);

    py::class_<BaseOperator>(m, "BaseOperator")
        .def("__call__", (void (BaseOperator::*)(SparseState &) const) & BaseOperator::operator())
        .def("dag", (void (BaseOperator::*)(SparseState &) const) & BaseOperator::dag);

    BIND_BASE_OPERATOR(SelfAdjointOperator)
#ifdef _WIN32
        .def("__call__", (void (SelfAdjointOperator::*)(SparseState &) const) & SelfAdjointOperator::operator())
#endif
        .def("dag", (void (SelfAdjointOperator::*)(SparseState &) const) & SelfAdjointOperator::dag);

    sparse_state.def_readonly("basis_states", &SparseState::basis_states)
        .def("size", &SparseState::size)
        .def("empty", &SparseState::empty);

    /* condrot.h */
    BIND_BASE_OPERATOR(CondRot_Rational_Bool)
        .def(py::init<std::string_view, std::string_view>())
        .def(py::init<size_t, size_t>());

    using CondRot_Rational_Bool_Cpp = CondRot_General_Bool<std::function<u22_t(size_t)>>;

    auto py_to_u22 = [](py::function py_func, size_t x) -> u22_t {
        py::object result = py_func(x);
        auto arr = result.cast<std::array<std::complex<double>, 4>>();
        return u22_t(arr);
    };

    BIND_BASE_OPERATOR_SUBNAME(CondRot_Rational_Bool_Cpp, CondRot_General_Bool)
        .def(py::init([py_to_u22](std::string_view reg_in, std::string_view reg_out, py::function py_func)
                      {
                auto cpp_func = [py_func, py_to_u22](size_t x) -> u22_t {
                    return py_to_u22(py_func, x);
                };
                return new CondRot_Rational_Bool_Cpp(reg_in, reg_out, cpp_func); }),
             py::arg("reg_in"), py::arg("reg_out"), py::arg("angle_function"))
        .def(py::init([py_to_u22](size_t reg_in, size_t reg_out, py::function py_func)
                      {
                auto cpp_func = [py_func, py_to_u22](size_t x) -> u22_t {
                    return py_to_u22(py_func, x);
                };
                return new CondRot_Rational_Bool_Cpp(reg_in, reg_out, cpp_func); }),
             py::arg("reg_in"), py::arg("reg_out"), py::arg("angle_function"))

        //.def_static("_is_diagonal", &CondRot_Rational_Bool_Cpp::_is_diagonal)
        //.def_static("_is_off_diagonal", &CondRot_Rational_Bool_Cpp::_is_off_diagonal)

        .def("operate_pair", &CondRot_Rational_Bool_Cpp::operate_pair)
        .def("operate_alone_zero", &CondRot_Rational_Bool_Cpp::operate_alone_zero)
        .def("operate_alone_one", &CondRot_Rational_Bool_Cpp::operate_alone_one);

    /* dark_magic.h */
    BIND_SELF_ADJOINT_OPERATOR(Normalize, R"doc(
Normalize the quantum state.

Ensures the state vector has unit norm by dividing all amplitudes
by the total norm. Call after operations that may leave the state
unnormalized.

Example:
    Normalize()(state)
)doc")
        .def(py::init<>());

    BIND_SELF_ADJOINT_OPERATOR(Init_Unsafe, R"doc(
Initialize a register to a specific value (unsafe).

Sets the register to a classical value without checking normalization.
Use with caution as it modifies amplitudes directly.

Args:
    reg: Register name (str) or ID (int).
    value: Classical value to set.

Example:
    Init_Unsafe("q", 5)(state)  # Set register q to value 5
)doc")
        .def(py::init<std::string_view, size_t>(),
             py::arg("reg"), py::arg("value"))
        .def(py::init<size_t, size_t>(),
             py::arg("id"), py::arg("value"));

    /* debugger.h */
    BIND_BASE_OPERATOR(ModuleInheritance_Test)
        .def(py::init<>());

    BIND_SELF_ADJOINT_OPERATOR(ModuleInheritance_Test_SelfAdjoint)
        .def(py::init<>());

    // 绑定枚举类型
    py::enum_<StatePrintDisplay>(m, "StatePrintDisplay")
        .value("Default", StatePrintDisplay::Default)
        .value("Detail", StatePrintDisplay::Detail)
        .value("Binary", StatePrintDisplay::Binary)
        .value("Prob", StatePrintDisplay::Prob)
        .export_values();

    // CheckNormalization 绑定
    BIND_SELF_ADJOINT_OPERATOR(CheckNormalization)
        .def(py::init<>())
        .def(py::init<double>(), py::arg("threshold"));

    // CheckNan 绑定
    BIND_SELF_ADJOINT_OPERATOR(CheckNan)
        .def(py::init<>());

    // ViewNormalization 绑定
    BIND_SELF_ADJOINT_OPERATOR(ViewNormalization)
        .def(py::init<>());

    // StatePrint 绑定
    BIND_SELF_ADJOINT_OPERATOR(StatePrint)
        .def(py::init<int32_t>(), py::arg("disp") = 0)
        .def(py::init<int32_t, int>(), py::arg("disp"), py::arg("precision"))
        .def(py::init<StatePrintDisplay>(), py::arg("disp"))
        .def_readwrite_static("on", &StatePrint::on);

    // TestRemovable 绑定
    BIND_SELF_ADJOINT_OPERATOR(TestRemovable)
        .def(py::init<std::string_view>(), py::arg("register_name"))
        .def(py::init<size_t>(), py::arg("register_id"));

    // CheckDuplicateKey 绑定
    BIND_SELF_ADJOINT_OPERATOR(CheckDuplicateKey)
        .def(py::init<>());

    /* hadamard.h */
    // 绑定Hadamard_Int
    BIND_SELF_ADJOINT_OPERATOR(Hadamard_Int, R"doc(
Apply Hadamard transform to an integer register.

Creates an equal superposition over all integer values from 0 to 2^n - 1
for the specified number of digits.

Args:
    reg_in: Name/ID of the input register.
    n_digits: Number of digits (qubits) to apply Hadamard to.

Example:
    Hadamard_Int("q", 4)(state)  # Superpose q over 0..15
)doc")
        .def(py::init<std::string_view, size_t>(), py::arg("reg_in"), py::arg("n_digits"))
        .def(py::init<size_t, size_t>(), py::arg("reg_in"), py::arg("n_digits"))
            BIND_CONTROLLABLE_METHODS(Hadamard_Int);

    // 绑定Hadamard_Int_Full
    BIND_SELF_ADJOINT_OPERATOR(Hadamard_Int_Full)
        .def(py::init<std::string_view>(), py::arg("reg_in"))
        .def(py::init<size_t>(), py::arg("reg_in"))
            BIND_CONTROLLABLE_METHODS(Hadamard_Int_Full);

    // 绑定Hadamard_Bool
    BIND_SELF_ADJOINT_OPERATOR(Hadamard_Bool)
        .def(py::init<std::string_view>(), py::arg("reg_in"))
        .def(py::init<size_t>(), py::arg("reg_in"))
            BIND_CONTROLLABLE_METHODS(Hadamard_Bool);

    // 绑定Hadamard_PartialQubit（需要特殊处理std::set参数）
    BIND_SELF_ADJOINT_OPERATOR(Hadamard_PartialQubit)
        .def(py::init([](std::string_view reg_in, py::set positions)
                      {
		std::set<size_t> pos_set;
		for (auto item : positions) {
			pos_set.insert(item.cast<size_t>());
		}
		return new Hadamard_PartialQubit(reg_in, pos_set); }),
             py::arg("reg_in"), py::arg("qubit_positions"))
        .def(py::init([](size_t reg_in, py::set positions)
                      {
		std::set<size_t> pos_set;
		for (auto item : positions) {
			pos_set.insert(item.cast<size_t>());
		}
		return new Hadamard_PartialQubit(reg_in, pos_set); }),
             py::arg("reg_in"), py::arg("qubit_positions"))
            BIND_CONTROLLABLE_METHODS(Hadamard_PartialQubit);

    /* parallel_phase_operations.h */
    BIND_SELF_ADJOINT_OPERATOR(ZeroConditionalPhaseFlip)
        .def(py::init<const std::vector<size_t> &>(), py::arg("reg_ids"))
        .def(py::init<const std::vector<std::string> &>(), py::arg("regs"))
            BIND_CONTROLLABLE_METHODS(ZeroConditionalPhaseFlip);

    BIND_SELF_ADJOINT_OPERATOR(Reflection_Bool)
        .def(py::init<std::string_view, bool>(), py::arg("reg"), py::arg("inverse") = false)
        .def(py::init<size_t, bool>(), py::arg("reg_id"), py::arg("inverse") = false)
        .def(py::init<const std::vector<size_t> &, bool>(), py::arg("reg_ids"), py::arg("inverse") = false)
        .def(py::init<const std::vector<std::string> &, bool>(), py::arg("regs"), py::arg("inverse") = false)
            BIND_CONTROLLABLE_METHODS(Reflection_Bool);

    BIND_BASE_OPERATOR(GlobalPhase_Int)
        .def(py::init<complex_t>(), py::arg("phase"))
            BIND_CONTROLLABLE_METHODS(GlobalPhase_Int);

    /* partial_trace.h */
    // PartialTrace 绑定
    py::class_<PartialTrace>(m, "PartialTrace")
        .def(py::init<const std::vector<std::string> &>(),
             py::arg("partial_trace_register_names"))
        .def(py::init<const std::vector<size_t> &>(),
             py::arg("partial_trace_register_ids"))
        .def(py::init<std::string>(),
             py::arg("single_register_name"))
        .def(py::init<size_t>(),
             py::arg("single_register_id"))
        .def("__call__",
             (std::pair<std::vector<uint64_t>, double> (PartialTrace::*)(SparseState &) const) & PartialTrace::operator(), py::arg("state"));

    // PartialTraceSelect 绑定
    py::class_<PartialTraceSelect>(m, "PartialTraceSelect")
        // 多版本构造函数
        .def(py::init<const std::map<std::string_view, uint64_t> &>(),
             py::arg("name_value_map"))
        .def(py::init<const std::map<size_t, uint64_t> &>(),
             py::arg("id_value_map"))
        .def(py::init<const std::vector<size_t> &, const std::vector<uint64_t> &>(),
             py::arg("reg_ids"), py::arg("select_values"))
        // 调用操作符
        .def("__call__",
             (double (PartialTraceSelect::*)(SparseState &) const) & PartialTraceSelect::operator(), py::arg("state"));

    // PartialTraceSelectRange 绑定
    py::class_<PartialTraceSelectRange>(m, "PartialTraceSelectRange")
        // 范围构造版本
        .def(py::init<std::string, std::pair<size_t, size_t>>(),
             py::arg("register_name"), py::arg("select_range"))
        .def(py::init<size_t, std::pair<size_t, size_t>>(),
             py::arg("register_id"), py::arg("select_range"))
        // 操作符绑定
        .def("__call__",
             (double (PartialTraceSelectRange::*)(SparseState &) const) & PartialTraceSelectRange::operator(), py::arg("state"));

    /* qft.h */
    // 绑定QFT
    BIND_BASE_OPERATOR(QFT, R"doc(
Quantum Fourier Transform on a register.

Applies the QFT to transform between computational and Fourier bases.
Commonly used in phase estimation and Shor's algorithm.

Args:
    reg_name: Name of the register to transform (str) or register ID (int).

Example:
    QFT("data")(state)  # Apply QFT
    # ... computation ...
    inverseQFT("data")(state)  # Apply inverse QFT
)doc")
        // 寄存器名称/ID构造
        .def(py::init<std::string_view>(), py::arg("reg_name"))
        .def(py::init<size_t>(), py::arg("reg_id"))
            BIND_CONTROLLABLE_METHODS(QFT);

    // 绑定inverseQFT（保持C++命名风格）
    BIND_BASE_OPERATOR(inverseQFT, R"doc(
Inverse Quantum Fourier Transform on a register.

Applies the inverse QFT to transform from Fourier basis back to
computational basis.

Args:
    reg_name: Name of the register (str) or register ID (int).
)doc")
        .def(py::init<std::string_view>(), py::arg("reg_name"))
        .def(py::init<size_t>(), py::arg("reg_id"))
            BIND_CONTROLLABLE_METHODS(inverseQFT);

    /* qram.h */
    py::class_<qram_qutrit::QRAMCircuit>(m, "QRAMCircuit_qutrit")
        .def(py::init<size_t, size_t>(), py::arg("addr_size"), py::arg("data_size"))
        .def(py::init<size_t, size_t, const memory_t &>(), py::arg("addr_size"), py::arg("data_size"), py::arg("memory"))
        .def(py::init<size_t, size_t, memory_t &&>(), py::arg("addr_size"), py::arg("data_size"), py::arg("memory"))
        .def_readonly("address_size", &qram_qutrit::QRAMCircuit::address_size)
        .def_readonly("data_size", &qram_qutrit::QRAMCircuit::data_size);

    /* hamiltonian_simulation.h - XOR-based self-adjoint address operators */
    using namespace CKS;
    BIND_SELF_ADJOINT_OPERATOR(GetRowAddr)
        .def(py::init<std::string_view, std::string_view, size_t, std::string_view>(),
             py::arg("reg_offset"), py::arg("reg_row"), py::arg("row_size"), py::arg("reg_row_offset"))
        .def(py::init<size_t, size_t, size_t, size_t>(),
             py::arg("reg_offset"), py::arg("reg_row"), py::arg("row_size"), py::arg("reg_row_offset"));

    BIND_SELF_ADJOINT_OPERATOR(GetDataAddr)
        .def(py::init<std::string_view, std::string_view, std::string_view, size_t, std::string_view>(),
             py::arg("reg_offset"), py::arg("reg_row"), py::arg("reg_col"), py::arg("row_size"), py::arg("reg_data_offset"))
        .def(py::init<size_t, size_t, size_t, size_t, size_t>(),
             py::arg("reg_offset"), py::arg("reg_row"), py::arg("reg_col"), py::arg("row_size"), py::arg("reg_data_offset"));

    // 绑定QRAMLoad
    BIND_SELF_ADJOINT_OPERATOR(QRAMLoad, R"doc(
Load classical data into quantum superposition via QRAM.

Performs the QRAM load operation, creating a superposition where each
basis state is entangled with its corresponding data value.

Args:
    qram: QRAMCircuit_qutrit instance containing the memory.
    addr_reg: Name/ID of the address register.
    data_reg: Name/ID of the data register.

Example:
    qram = QRAMCircuit_qutrit(addr_size=3, data_size=4, memory=data)
    QRAMLoad(qram, "address", "data")(state)

Note:
    Use QRAMLoadFast for optimized execution when address distribution
    is uniform.
)doc")
        // 构造函数（处理寄存器名称到ID的转换）
        .def(py::init<qram_qutrit::QRAMCircuit *, std::string_view, std::string_view>(),
             py::arg("qram"), py::arg("addr_reg"), py::arg("data_reg"))
        .def(py::init<qram_qutrit::QRAMCircuit *, size_t, size_t>(),
             py::arg("qram"), py::arg("addr_reg_id"), py::arg("data_reg_id"))

        // 可控方法绑定
        BIND_CONTROLLABLE_METHODS(QRAMLoad)

        // 核心属性
        .def_readonly("qram_circuit", &QRAMLoad::qram)
        .def_readonly_static("version", &QRAMLoad::version);

    // 绑定QRAMLoadFast
    BIND_SELF_ADJOINT_OPERATOR(QRAMLoadFast)
        // 构造方法（复用相同的内存管理策略）
        .def(py::init<qram_qutrit::QRAMCircuit *, std::string_view, std::string_view>(),
             py::arg("qram"), py::arg("addr_reg"), py::arg("data_reg"))
        .def(py::init<qram_qutrit::QRAMCircuit *, size_t, size_t>(),
             py::arg("qram"), py::arg("addr_reg_id"), py::arg("data_reg_id"))

        // 可控方法绑定（不含SELF_ADJOINT）
        BIND_CONTROLLABLE_METHODS(QRAMLoadFast);

    // 基础算术操作绑定
    BIND_SELF_ADJOINT_OPERATOR(Xgate_Bool)
    BIND_CONTROLLABLE_METHODS(Xgate_Bool)
        .def(py::init<std::string_view, size_t>(),
             py::arg("reg"), py::arg("digit"))
        .def(py::init<size_t, size_t>(),
             py::arg("reg_id"), py::arg("digit"));

    BIND_SELF_ADJOINT_OPERATOR(FlipBools)
    BIND_CONTROLLABLE_METHODS(FlipBools)
        .def(py::init<std::string_view>(), py::arg("reg"))
        .def(py::init<size_t>(), py::arg("reg_id"));

    // 交换操作绑定
    BIND_SELF_ADJOINT_OPERATOR(Swap_Bool_Bool)
        .def(py::init<std::string_view, size_t, std::string_view, size_t>(),
             py::arg("reg1"), py::arg("digit1"), py::arg("reg2"), py::arg("digit2"))
        .def(py::init<size_t, size_t, size_t, size_t>(),
             py::arg("reg1_id"), py::arg("digit1"), py::arg("reg2_id"), py::arg("digit2"))

            BIND_CONTROLLABLE_METHODS(Swap_Bool_Bool);

    // 位移操作
    BIND_BASE_OPERATOR(ShiftLeft)
        .def(py::init<std::string_view, size_t>(),
             py::arg("reg"), py::arg("shift_bits"))
        .def(py::init<size_t, size_t>(),
             py::arg("reg_id"), py::arg("shift_bits"))

            BIND_CONTROLLABLE_METHODS(ShiftLeft);

    BIND_BASE_OPERATOR(ShiftRight)
        .def(py::init<std::string_view, size_t>(),
             py::arg("reg"), py::arg("shift_bits"))
        .def(py::init<size_t, size_t>(),
             py::arg("reg_id"), py::arg("shift_bits"))

            BIND_CONTROLLABLE_METHODS(ShiftRight);

    // 算术运算绑定
    BIND_SELF_ADJOINT_OPERATOR(Mult_UInt_ConstUInt)
        .def(py::init<std::string_view, size_t, std::string_view>(),
             py::arg("input_reg"), py::arg("multiplier"), py::arg("output_reg"))
        .def(py::init<size_t, size_t, size_t>(),
             py::arg("input_id"), py::arg("multiplier"), py::arg("output_id"))

            BIND_CONTROLLABLE_METHODS(Mult_UInt_ConstUInt);

    BIND_BASE_OPERATOR(Add_Mult_UInt_ConstUInt)
        .def(py::init<std::string_view, size_t, std::string_view>(),
             py::arg("input_reg"), py::arg("multiplier"), py::arg("output_reg"))
        .def(py::init<size_t, size_t, size_t>(),
             py::arg("input_reg"), py::arg("multiplier"), py::arg("output_reg"))
            BIND_DAG_METHODS(Add_Mult_UInt_ConstUInt)
                BIND_CONTROLLABLE_METHODS(Add_Mult_UInt_ConstUInt);

    BIND_SELF_ADJOINT_OPERATOR(Add_UInt_UInt, R"doc(
Add two unsigned integer registers.

Computes: |a⟩|b⟩|0⟩ → |a⟩|b⟩|a+b⟩ (mod 2^n)

Args:
    input_reg1: Name/ID of the first input register (addend).
    input_reg2: Name/ID of the second input register (addend).
    output_reg: Name/ID of the output register (accumulates sum).

Example:
    Add_UInt_UInt("a", "b", "result")(state)  # result = a + b
)doc")
        .def(py::init<std::string_view, std::string_view, std::string_view>(),
             py::arg("input_reg1"), py::arg("input_reg2"), py::arg("output_reg"))
        .def(py::init<size_t, size_t, size_t>(),
             py::arg("input_id1"), py::arg("input_id2"), py::arg("output_id"))

            BIND_CONTROLLABLE_METHODS(Add_UInt_UInt);

    BIND_BASE_OPERATOR(Add_UInt_UInt_InPlace)
        .def(py::init<std::string_view, std::string_view>(),
             py::arg("input_reg"), py::arg("output_reg"))
        .def(py::init<size_t, size_t>(),
             py::arg("input_reg"), py::arg("output_reg"))
            BIND_DAG_METHODS(Add_UInt_UInt_InPlace)
                BIND_CONTROLLABLE_METHODS(Add_UInt_UInt_InPlace);

    BIND_SELF_ADJOINT_OPERATOR(Add_UInt_ConstUInt)
        .def(py::init<std::string_view, size_t, std::string_view>(),
             py::arg("input_reg"), py::arg("add"), py::arg("output_reg"))
        .def(py::init<size_t, size_t, size_t>(),
             py::arg("input_reg"), py::arg("add"), py::arg("output_reg"))

            BIND_CONTROLLABLE_METHODS(Add_UInt_ConstUInt);

    BIND_BASE_OPERATOR(Add_ConstUInt)
        .def(py::init<std::string_view, size_t>(),
             py::arg("input_reg"), py::arg("add"))
        .def(py::init<size_t, size_t>(),
             py::arg("input_reg"), py::arg("add"))

            BIND_CONTROLLABLE_METHODS(Add_ConstUInt);

    // 复杂算术操作
    BIND_SELF_ADJOINT_OPERATOR(Div_Sqrt_Arccos_Int_Int)
        .def(py::init<std::string_view, std::string_view, std::string_view>(),
             py::arg("lhs_reg"), py::arg("rhs_reg"), py::arg("out_reg"))
        .def(py::init<size_t, size_t, size_t>(),
             py::arg("lhs_reg"), py::arg("rhs_reg"), py::arg("out_reg"))

            BIND_CONTROLLABLE_METHODS(Div_Sqrt_Arccos_Int_Int);

    BIND_SELF_ADJOINT_OPERATOR(Sqrt_Div_Arccos_Int_Int)
        .def(py::init<std::string_view, std::string_view, std::string_view>(),
             py::arg("lhs_reg"), py::arg("rhs_reg"), py::arg("out_reg"))
        .def(py::init<size_t, size_t, size_t>(),
             py::arg("lhs_reg"), py::arg("rhs_reg"), py::arg("out_reg"))

            BIND_CONTROLLABLE_METHODS(Sqrt_Div_Arccos_Int_Int);

    BIND_SELF_ADJOINT_OPERATOR(GetRotateAngle_Int_Int)
        .def(py::init<std::string_view, std::string_view, std::string_view>(),
             py::arg("lhs_reg"), py::arg("rhs_reg"), py::arg("out_reg"))
        .def(py::init<size_t, size_t, size_t>(),
             py::arg("lhs_reg"), py::arg("rhs_reg"), py::arg("out_reg"))

            BIND_CONTROLLABLE_METHODS(GetRotateAngle_Int_Int);

    BIND_BASE_OPERATOR(AddAssign_AnyInt_AnyInt)
        .def(py::init<std::string_view, std::string_view>(),
             py::arg("input_reg"), py::arg("output_reg"))
        .def(py::init<size_t, size_t>(),
             py::arg("input_reg"), py::arg("output_reg"))
            BIND_DAG_METHODS(AddAssign_AnyInt_AnyInt)
                BIND_CONTROLLABLE_METHODS(AddAssign_AnyInt_AnyInt);

    // 通用赋值操作
    BIND_SELF_ADJOINT_OPERATOR(Assign)
        .def(py::init<std::string_view, std::string_view>(), py::arg("src"), py::arg("dst"))
        .def(py::init<size_t, size_t>(), py::arg("src_id"), py::arg("dst_id"))

            BIND_CONTROLLABLE_METHODS(Assign);

    // 比较操作绑定
    BIND_SELF_ADJOINT_OPERATOR(Compare_UInt_UInt)
        .def(py::init<std::string_view, std::string_view, std::string_view, std::string_view>(),
             py::arg("left_reg"), py::arg("right_reg"),
             py::arg("less_flag_reg"), py::arg("equal_flag_reg"))
        .def(py::init<size_t, size_t, size_t, size_t>(),
             py::arg("left_id"), py::arg("right_id"),
             py::arg("less_flag_id"), py::arg("equal_flag_id"))

            BIND_CONTROLLABLE_METHODS(Compare_UInt_UInt);

    // 小于比较绑定
    BIND_SELF_ADJOINT_OPERATOR(Less_UInt_UInt)
        .def(py::init<std::string_view, std::string_view, std::string_view>(),
             py::arg("left_reg"), py::arg("right_reg"), py::arg("less_flag_reg"))
        .def(py::init<size_t, size_t, size_t>(),
             py::arg("left_id"), py::arg("right_id"), py::arg("less_flag_id"))

            BIND_CONTROLLABLE_METHODS(Less_UInt_UInt);

    // 通用寄存器交换绑定
    BIND_SELF_ADJOINT_OPERATOR(Swap_General_General)
        .def(py::init([](std::string_view reg1, std::string_view reg2)
                      {
		// 禁止交换同一寄存器
		if (reg1 == reg2)
			throw std::invalid_argument("Cannot swap the same register");
		return new Swap_General_General(System::get(reg1), System::get(reg2)); }),
             py::arg("reg1"), py::arg("reg2"))
        .def(py::init<size_t, size_t>(), py::arg("reg1_id"), py::arg("reg2_id"))

            BIND_CONTROLLABLE_METHODS(Swap_General_General);

    // 中值计算绑定
    BIND_SELF_ADJOINT_OPERATOR(GetMid_UInt_UInt)
        .def(py::init<std::string_view, std::string_view, std::string_view>(),
             py::arg("left_reg"), py::arg("right_reg"), py::arg("mid_reg"))
        .def(py::init<size_t, size_t, size_t>(),
             py::arg("left_id"), py::arg("right_id"), py::arg("mid_id"))

            BIND_CONTROLLABLE_METHODS(GetMid_UInt_UInt);

    // 通用算术运算符
    BIND_SELF_ADJOINT_OPERATOR(CustomArithmetic)
        .def(py::init([](py::list input_registers, py::int_ input_size, py::int_ output_size, py::function func)
                      {
				/* Two inputs are allowed, std::vector<size_t> or std::vector<std::string> */
				std::vector<size_t> input_ids;
				for (auto item : input_registers) {
					if (py::isinstance<py::str>(item)) {
						input_ids.push_back(System::get(item.cast<std::string>()));
					}
					else if (py::isinstance<py::int_>(item)) {
						input_ids.push_back(item.cast<size_t>());
					}
					else {
						throw std::invalid_argument("Input registers must be either string or integer");
					}
				}

				/* Cast function into GenericArithmetic type */
				GenericArithmetic func_cpp = [func](const std::vector<size_t>& inputs) -> std::vector<size_t> {
					return func(inputs).cast<std::vector<size_t>>();
				};

				return new CustomArithmetic(input_ids, input_size.cast<size_t>(), output_size.cast<size_t>(), func_cpp); }),
             py::arg("input_registers"), py::arg("input_size"), py::arg("output_size"), py::arg("func"))
	    BIND_CONTROLLABLE_METHODS(CustomArithmetic);

    /* quantum_interfere_basic.h */
    // 哈希函数对象绑定
    py::class_<StateHashExceptKey>(m, "StateHashExceptKey")
        .def(py::init<size_t>(), py::arg("excluded_id"))
        .def("__call__", &StateHashExceptKey::operator());

    py::class_<StateHashExceptQubits>(m, "StateHashExceptQubits")
        .def(py::init<size_t, std::set<size_t>>(),
             py::arg("target_id"), py::arg("excluded_qubits"))
        .def("__call__", &StateHashExceptQubits::operator());

    // 等价比较器绑定
    py::class_<StateEqualExceptKey>(m, "StateEqualExceptKey")
        .def(py::init<size_t>(), py::arg("excluded_id"))
        .def("__call__", &StateEqualExceptKey::operator());

    py::class_<StateEqualExceptQubits>(m, "StateEqualExceptQubits")
        .def(py::init<size_t, std::set<size_t>>(),
             py::arg("target_id"), py::arg("excluded_qubits"))
        .def("__call__", &StateEqualExceptQubits::operator());

    // 排序比较器绑定
    py::class_<StateLessExceptKey>(m, "StateLessExceptKey")
        .def(py::init<size_t>(), py::arg("excluded_id"))
        .def("__call__", &StateLessExceptKey::operator());

    py::class_<StateLessExceptQubits>(m, "StateLessExceptQubits")
        .def(py::init<size_t, std::set<size_t>>(),
             py::arg("target_id"), py::arg("excluded_qubits"))
        .def("__call__", &StateLessExceptQubits::operator());

    /* rot.h */
    // 通用旋转绑定
    // BIND_BASE_OPERATOR(Rot_General_Bool)
    //	.def(py::init<std::string_view, size_t, std::array<std::complex<double>, 4>>(),
    //		py::arg("reg"), py::arg("digit"), py::arg("matrix"))
    //	.def(py::init<int, size_t, std::array<std::complex<double>, 4>>(),
    //		py::arg("reg_id"), py::arg("digit"), py::arg("matrix"))
    //	BIND_CONTROLLABLE_METHODS(Rot_General_Bool);

    //// 基本布尔旋转绑定
    // BIND_BASE_OPERATOR(Rot_Bool)
    //	.def(py::init<std::string_view, std::array<std::complex<double>, 4>>(),
    //		py::arg("reg"), py::arg("matrix"))
    //	.def(py::init<int, std::array<std::complex<double>, 4>>(),
    //		py::arg("reg_id"), py::arg("matrix"))
    //	BIND_CONTROLLABLE_METHODS(Rot_Bool);

    // 通用酉矩阵绑定
    BIND_BASE_OPERATOR(Rot_GeneralUnitary)
        .def(py::init<std::string_view, const DenseMatrix<complex_t> &>(),
             py::arg("reg"), py::arg("unitary_matrix"))
        .def(py::init<size_t, const DenseMatrix<complex_t> &>(),
             py::arg("reg_id"), py::arg("unitary_matrix"))
            BIND_CONTROLLABLE_METHODS(Rot_GeneralUnitary);

    // 状态准备绑定
    BIND_BASE_OPERATOR(Rot_GeneralStatePrep)
        .def(py::init<std::string_view, const std::vector<std::complex<double>> &>(),
             py::arg("reg"), py::arg("state_vector"))
        .def(py::init<size_t, const std::vector<std::complex<double>> &>(),
             py::arg("reg_id"), py::arg("state_vector"));

    // 辅助函数绑定
    m.def("stateprep_unitary_build_schmidt", &stateprep_unitary_build_schmidt,
          py::arg("state_vector"), "Build unitary for state preparation");

    /* sort_state.h */
    // 排序操作绑定
    BIND_SELF_ADJOINT_OPERATOR(SortExceptKey)
        .def(py::init<std::string_view>(), py::arg("key"))
        .def(py::init<size_t>(), py::arg("key_id"));

    BIND_SELF_ADJOINT_OPERATOR(SortByKey)
        .def(py::init<std::string_view>(), py::arg("key"))
        .def(py::init<size_t>(), py::arg("key_id"));

    BIND_SELF_ADJOINT_OPERATOR(SortExceptBit)
        .def(py::init<std::string_view, size_t>(), py::arg("key"), py::arg("digit"))
        .def(py::init<size_t, size_t>(), py::arg("key_id"), py::arg("digit"));

    BIND_SELF_ADJOINT_OPERATOR(SortExceptKeyHadamard)
        .def(py::init<std::string_view, std::set<size_t>>(),
             py::arg("key"), py::arg("qubit_ids"))
        .def(py::init<size_t, std::set<size_t>>(),
             py::arg("key_id"), py::arg("qubit_ids"));

    BIND_SELF_ADJOINT_OPERATOR(SortUnconditional)
        .def(py::init<>());

    BIND_SELF_ADJOINT_OPERATOR(SortByAmplitude)
        .def(py::init<>());

    BIND_SELF_ADJOINT_OPERATOR(SortByKey2)
        .def(py::init<std::string_view, std::string_view>(),
             py::arg("key1"), py::arg("key2"))
        .def(py::init<size_t, size_t>(),
             py::arg("key1_id"), py::arg("key2_id"));

    /* system_operations.h */
    // 系统分割组合

    m.def("split_systems", (SparseState (*)(SparseState &state, const std::vector<size_t> &, const std::vector<size_t> &, const std::vector<std::pair<size_t, size_t>> &, const std::vector<std::pair<size_t, size_t>> &))&split_systems,
          py::arg("state"),
          py::arg("condition_variable_nonzeros"),
          py::arg("condition_variable_all_ones"),
          py::arg("condition_variable_by_bit"),
          py::arg("condition_variable_by_value"));

    m.def("combine_systems", (void (*)(SparseState &, const SparseState &))&combine_systems,
          py::arg("to"), py::arg("from_"));

    // 寄存器操作
    py::class_<SplitRegister>(m, "SplitRegister")
        .def(py::init<std::string_view, std::string_view, size_t>(),
             py::arg("first"), py::arg("second"), py::arg("size"))
        .def("__call__", (size_t (SplitRegister::*)(SparseState &) const) & SplitRegister::operator());

    py::class_<CombineRegister>(m, "CombineRegister")
        .def(py::init<std::string_view, std::string_view>(),
             py::arg("first"), py::arg("second"))
        .def("__call__", (size_t (CombineRegister::*)(SparseState &) const) & CombineRegister::operator());

    //// 系统重置
    // py::class_<ResetSystems>(m, "ResetSystems")
    //	.def(py::init<>())
    //	.def("__call__", &ResetSystems::operator());

    // 寄存器管理
    py::class_<MoveBackRegister>(m, "MoveBackRegister")
        .def(py::init<std::string_view>(), py::arg("reg"))
        .def(py::init<size_t>(), py::arg("reg_id"))
        .def("__call__", (void (MoveBackRegister::*)(SparseState &) const) & MoveBackRegister::operator());

    py::class_<AddRegister>(m, "AddRegister")
        .def(py::init<std::string_view, StateStorageType, size_t>(),
             py::arg("name"), py::arg("type"), py::arg("size"))
        .def("__call__", (size_t (AddRegister::*)(SparseState &) const) & AddRegister::operator());

    py::class_<AddRegisterWithHadamard>(m, "AddRegisterWithHadamard")
        .def(py::init<std::string_view, StateStorageType, size_t>(),
             py::arg("name"), py::arg("type"), py::arg("size"))
        .def("__call__", (size_t (AddRegisterWithHadamard::*)(SparseState &) const) & AddRegisterWithHadamard::operator());

    py::class_<RemoveRegister>(m, "RemoveRegister")
        .def(py::init<std::string_view>(), py::arg("name"))
        .def(py::init<size_t>(), py::arg("reg_id"))
        .def("__call__", (void (RemoveRegister::*)(SparseState &) const) & RemoveRegister::operator());

    // 栈操作
    BIND_BASE_OPERATOR(Push)
        .def(py::init<std::string_view, std::string_view>(),
             py::arg("reg"), py::arg("garbage"))
        .def(py::init<size_t, std::string_view>(),
             py::arg("reg_id"), py::arg("garbage"));

    BIND_BASE_OPERATOR(Pop)
        .def(py::init<std::string_view>(), py::arg("reg"))
        .def(py::init<size_t>(), py::arg("reg_id"));

    // 状态清理
    BIND_SELF_ADJOINT_OPERATOR(ClearZero)
        .def(py::init<>())
        .def(py::init<double>(), py::arg("epsilon"));

    BIND_BASE_OPERATOR(Phase_Bool)
        .def(py::init<std::string_view, size_t, double>(),
             py::arg("reg"), py::arg("digit"), py::arg("lambda_"))
        .def(py::init<size_t, size_t, double>(),
             py::arg("reg_id"), py::arg("digit"), py::arg("lambda_"))
        .def(py::init<std::string_view, double>(),
             py::arg("reg"), py::arg("lambda_"))
        .def(py::init<size_t, double>(),
             py::arg("reg_id"), py::arg("lambda_"))
            BIND_CONTROLLABLE_METHODS(Phase_Bool);

    BIND_BASE_OPERATOR(Rot_Bool)
        .def(py::init<std::string_view, size_t, u22_t>(),
             py::arg("reg"), py::arg("digit"), py::arg("matrix"))
        .def(py::init<size_t, size_t, u22_t>(),
             py::arg("reg_id"), py::arg("digit"), py::arg("matrix"))
        .def(py::init<std::string_view, u22_t>(),
             py::arg("reg"), py::arg("matrix"))
        .def(py::init<size_t, u22_t>(),
             py::arg("reg_id"), py::arg("matrix"))
            BIND_CONTROLLABLE_METHODS(Rot_Bool);

    BIND_BASE_OPERATOR(Ygate_Bool)
        .def(py::init<std::string_view, size_t>(),
             py::arg("reg"), py::arg("digit") = 0)
        .def(py::init<size_t, size_t>(),
             py::arg("reg_id"), py::arg("digit") = 0)
            BIND_CONTROLLABLE_METHODS(Ygate_Bool);

    py::class_<Zgate_Bool, Phase_Bool>(m, "Zgate_Bool")
        .def(py::init<std::string_view, size_t>(),
             py::arg("reg"), py::arg("digit") = 0)
        .def(py::init<size_t, size_t>(),
             py::arg("reg_id"), py::arg("digit") = 0);

    py::class_<Sgate_Bool, Phase_Bool>(m, "Sgate_Bool")
        .def(py::init<std::string_view, size_t>(),
             py::arg("reg"), py::arg("digit") = 0)
        .def(py::init<size_t, size_t>(),
             py::arg("reg_id"), py::arg("digit") = 0);

    py::class_<Tgate_Bool, Phase_Bool>(m, "Tgate_Bool")
        .def(py::init<std::string_view, size_t>(),
             py::arg("reg"), py::arg("digit") = 0)
        .def(py::init<size_t, size_t>(),
             py::arg("reg_id"), py::arg("digit") = 0);

    py::class_<RXgate_Bool, Rot_Bool>(m, "RXgate_Bool")
        .def(py::init<std::string_view, size_t, double>(),
             py::arg("reg"), py::arg("digit"), py::arg("theta"))
        .def(py::init<size_t, size_t, double>(),
             py::arg("reg_id"), py::arg("digit"), py::arg("theta"))
        .def(py::init<std::string_view, double>(),
             py::arg("reg"), py::arg("theta"))
        .def(py::init<size_t, double>(),
             py::arg("reg_id"), py::arg("theta"));

    py::class_<RYgate_Bool, Rot_Bool>(m, "RYgate_Bool")
        .def(py::init<std::string_view, size_t, double>(),
             py::arg("reg"), py::arg("digit"), py::arg("theta"))
        .def(py::init<size_t, size_t, double>(),
             py::arg("reg_id"), py::arg("digit"), py::arg("theta"))
        .def(py::init<std::string_view, double>(),
             py::arg("reg"), py::arg("theta"))
        .def(py::init<size_t, double>(),
             py::arg("reg_id"), py::arg("theta"));

    BIND_BASE_OPERATOR(RZgate_Bool)
        .def(py::init<std::string_view, size_t, double>(),
             py::arg("reg"), py::arg("digit"), py::arg("theta"))
        .def(py::init<size_t, size_t, double>(),
             py::arg("reg_id"), py::arg("digit"), py::arg("theta"))
        .def(py::init<std::string_view, double>(),
             py::arg("reg"), py::arg("theta"))
        .def(py::init<size_t, double>(),
             py::arg("reg_id"), py::arg("theta"))
            BIND_CONTROLLABLE_METHODS(RZgate_Bool);

    py::class_<SXgate_Bool, Rot_Bool>(m, "SXgate_Bool")
        .def(py::init<std::string_view, size_t>(),
             py::arg("reg"), py::arg("digit") = 0)
        .def(py::init<size_t, size_t>(),
             py::arg("reg_id"), py::arg("digit") = 0);

    py::class_<U2gate_Bool, Rot_Bool>(m, "U2gate_Bool")
        .def(py::init<std::string_view, size_t, double, double>(),
             py::arg("reg"), py::arg("digit"), py::arg("phi"), py::arg("lambda_"))
        .def(py::init<size_t, size_t, double, double>(),
             py::arg("reg_id"), py::arg("digit"), py::arg("phi"), py::arg("lambda_"))
        .def(py::init<std::string_view, double, double>(),
             py::arg("reg"), py::arg("phi"), py::arg("lambda_"))
        .def(py::init<size_t, double, double>(),
             py::arg("reg_id"), py::arg("phi"), py::arg("lambda_"));

    py::class_<U3gate_Bool, Rot_Bool>(m, "U3gate_Bool")
        .def(py::init<std::string_view, size_t, double, double, double>(),
             py::arg("reg"), py::arg("digit"), py::arg("theta"), py::arg("phi"), py::arg("lambda_"))
        .def(py::init<size_t, size_t, double, double, double>(),
             py::arg("reg_id"), py::arg("digit"), py::arg("theta"), py::arg("phi"), py::arg("lambda_"))
        .def(py::init<std::string_view, double, double, double>(),
             py::arg("reg"), py::arg("theta"), py::arg("phi"), py::arg("lambda_"))
        .def(py::init<size_t, double, double, double>(),
             py::arg("reg_id"), py::arg("theta"), py::arg("phi"), py::arg("lambda_"));
    {
         using namespace block_encoding;
         {
              using namespace block_encoding_tridiagonal;
              BIND_BASE_OPERATOR(PlusOneAndOverflow)
                  .def(py::init<std::string_view, std::string_view>(),
                       py::arg("main_reg"), py::arg("overflow"))
                      BIND_CONTROLLABLE_METHODS(PlusOneAndOverflow);
         }
    }

    // {
    //      using namespace CKS;

    //      /*py::class_<walk_angle_function_t>(m, "WalkAngleFunction")
    //           .def(py::init<>())
    //           .def("__call__", &walk_angle_function_t::operator())
    //           ;

    //      m.def("CKS_make_func", &make_func, py::arg("mat"));
    //      m.def("CKS_make_func_inv", &make_func_inv, py::arg("mat"));

    //      BIND_BASE_OPERATOR(CondRot_General_Bool_QW)
    //           .def(py::init(
    //                [](py::str j, py::str k, py::str reg_in, py::str reg_out, py::function pyfunc)
    //                {
    //                     auto j_str = j.cast<std::string>();
    //                     auto k_str = k.cast<std::string>();
    //                     auto reg_in_str = reg_in.cast<std::string>();
    //                     auto reg_out_str = reg_out.cast<std::string>();

    //                     auto cpp_func = [pyfunc](size_t value, size_t row_id, size_t col_id) -> u22_t
    //                          {
    //                               py::gil_scoped_acquire acquire;
    //                               return pyfunc(value, row_id, col_id).cast<u22_t>();
    //                          };
    //                     return new CondRot_General_Bool_QW(j_str, k_str, reg_in_str, reg_out_str, cpp_func);
    //                }
    //           ))
    //           ;*/

    //      BIND_SELF_ADJOINT_OPERATOR(QuantumBinarySearch)
    //          .def(py::init<qram_qutrit::QRAMCircuit *, std::string_view, size_t, std::string_view, std::string_view>(),
    //               py::arg("qram"), py::arg("column_index"), py::arg("address_offset_register_name"), py::arg("target_register_name"), py::arg("result_register_name"))
    //          .def(py::init<qram_qutrit::QRAMCircuit *, size_t, size_t, size_t, size_t>(),
    //               py::arg("qram"), py::arg("column_index"), py::arg("address_offset_register_id"), py::arg("target_register_id"), py::arg("result_register_id"));

    //      BIND_SELF_ADJOINT_OPERATOR(QuantumBinarySearchFast)
    //          .def(py::init<qram_qutrit::QRAMCircuit *, std::string_view, size_t, std::string_view, std::string_view>(),
    //               py::arg("qram"), py::arg("column_index"), py::arg("address_offset_register_name"), py::arg("target_register_name"), py::arg("result_register_name"))
    //          .def(py::init<qram_qutrit::QRAMCircuit *, size_t, size_t, size_t, size_t>(),
    //               py::arg("qram"), py::arg("column_index"), py::arg("address_offset_register_id"), py::arg("target_register_id"), py::arg("result_register_id"));

    //      BIND_SELF_ADJOINT_OPERATOR(GetRowAddr)
    //          .def(py::init<std::string_view, std::string_view, size_t, std::string_view>(),
    //               py::arg("reg_offset"), py::arg("reg_row"), py::arg("row_size"), py::arg("reg_row_offset"))
    //          .def(py::init<size_t, size_t, size_t, size_t>(),
    //               py::arg("reg_offset"), py::arg("reg_row"), py::arg("row_size"), py::arg("reg_row_offset"));

    //      BIND_SELF_ADJOINT_OPERATOR(GetDataAddr)
    //          .def(py::init<std::string_view, std::string_view, std::string_view, size_t, std::string_view>(),
    //               py::arg("reg_offset"), py::arg("reg_row"), py::arg("reg_col_sparse"), py::arg("row_sz"), py::arg("reg_data_offset"))
    //          .def(py::init<size_t, size_t, size_t, size_t, size_t>(),
    //               py::arg("reg_offset"), py::arg("reg_row"), py::arg("reg_col_sparse"), py::arg("row_sz"), py::arg("reg_data_offset"));
    // }
}

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif