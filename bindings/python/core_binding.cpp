// QRAM-Simulator C++ 核心的 pybind11 薄绑定层。
//
// 导出两类 QRAM 架构（qubit / qutrit）的 QRAMCircuit、全振幅桥接器
// QRAMFullAmp、时序与噪声调度器 TimeStep/TimeSlices/Operation(Pack)
// 以及全局随机种子控制，供外部 Python 库（如 SparQSim）与脚本化
// 实验直接驱动完整的"构造 → 设内存 → 设噪声 → 运行 → 保真度"工作流。

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
QRAM-Simulator C++ 核心的 Python 绑定。

提供 qubit / qutrit 两种 QRAM 架构的分支化稀疏仿真
(QRAMCircuitQubit / QRAMCircuitQutrit)、全振幅态向量桥接器
(QRAMFullAmp)、时序与噪声调度器 (TimeStep)，以及全局随机种子控制
(set_seed / get_seed)。

示例:

    from qram_simulator import QRAMCircuitQubit, OperationType, set_seed

    set_seed(42)
    qram = QRAMCircuitQubit(4, 2)
    qram.set_memory_random()
    qram.set_noise_models({OperationType.Depolarizing: 1e-3})
    qram.set_input_uniform(100)
    qram.run_normal()
    print(qram.sample_and_get_fidelity())
)doc";

	// ---- 全局随机种子控制（random_engine 单例）----
	m.def("set_seed",
		[](long long seed) { random_engine::set_seed(static_cast<seed_t>(seed)); },
		py::arg("seed"),
		"设置全局随机引擎（MT19937-64 单例）的种子。\n"
		"QRAMCircuit 的内存生成、输入采样与输出采样均使用该引擎，"
		"固定种子即可复现整条仿真链路。");
	m.def("get_seed",
		[]() { return static_cast<long long>(random_engine::get_seed()); },
		"返回全局随机引擎的当前种子。");

	// ---- 噪声参数键类型与架构常量 ----
	py::enum_<OperationType>(m, "OperationType", R"doc(
QRAM 操作类型枚举。

既是噪声模型的键类型（set_noise_models 的字典键），也是
Operation 的类型标签。噪声成员（BitFlip / PhaseFlip / BitPhaseFlip /
Depolarizing / Damping 等）取值为发生概率，须在 [0, 1] 内，
其中 Damping 要求 gamma < 1（gamma = 1 会在一步内衰灭全部激发，
破坏采样与归一化）。
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

	// ---- 调度数据结构（轻量只读视图）----
	py::class_<Operation>(m, "Operation", "单个量子操作：类型、目标比特与可选系数（噪声参数）。")
		.def_readonly("type", &Operation::type, "操作类型 (OperationType)。")
		.def_readonly("targets", &Operation::targets, "目标比特 / 节点编号列表。")
		.def_readonly("coefficients", &Operation::coefficients, "噪声系数（如 Damping 的 gamma）。")
		.def_readonly("dagger", &Operation::dagger, "是否为共轭转置（逆）操作。")
		.def("reverse", &Operation::reverse, "返回本操作的逆操作。")
		.def("to_string", &Operation::to_string, "返回可读字符串表示。")
		.def("__str__", &Operation::to_string);

	py::class_<OperationPack>(m, "OperationPack", "同一时间片内并发执行的操作集合（以 name 标识）。")
		.def_readwrite("name", &OperationPack::name, "本操作组的名称。")
		.def("reverse", &OperationPack::reverse, "返回逆操作组（按原顺序逐个取逆后重排）。")
		.def("empty", &OperationPack::empty, "本操作组是否为空。")
		.def("to_string", &OperationPack::to_string, "返回可读字符串表示。")
		.def("__str__", &OperationPack::to_string);

	py::class_<TimeSlices>(m, "TimeSlices", "完整 QRAM 装载调度：按时间片顺序排列的 OperationPack 序列。")
		.def_readwrite("time_slices", &TimeSlices::time_slices,
			"时间片列表（list[OperationPack]；读取为拷贝）。")
		.def("reverse", &TimeSlices::reverse, "返回逆调度（整体反序并逐组取逆）。")
		.def("clear", &TimeSlices::clear, "清空调度。")
		.def("to_string", &TimeSlices::to_string, "返回可读字符串表示。")
		.def("__str__", &TimeSlices::to_string)
		.def("__len__", [](const TimeSlices& self) { return self.time_slices.size(); },
			"时间片数量。");

	// ---- TimeStep：时序与噪声调度器 ----
	py::class_<TimeStep>(m, "TimeStep", R"doc(
QRAM 装载电路的时序与噪声调度器。

依据地址 / 数据宽度推导各时间片的路由、拷贝与交换操作，并可按
噪声模型在每个时间片后插入噪声操作，生成完整的 TimeSlices 调度。
QRAMCircuit 内部持有 TimeStep 实例并在 initialize_system 时调用
generate 生成含噪操作序列。
)doc")
		.def(py::init<size_t, size_t>(), py::arg("addr_size"), py::arg("data_size"),
			"构造调度器。\n\n参数:\n    addr_size: 地址位宽\n    data_size: 数据位宽")
		.def_readonly("addr_size", &TimeStep::addr_size, "地址位宽。")
		.def_readonly("data_size", &TimeStep::data_size, "数据位宽。")
		.def_readwrite("time_slices_noise_free", &TimeStep::time_slices_noise_free,
			"无噪调度（TimeSlices）。")
		.def("full_step", &TimeStep::full_step, "完整装载流程的总时间片数。")
		.def("generate_step", &TimeStep::generate_step, py::arg("step"),
			"生成指定时间片的操作组（不含噪声）。\n\n参数:\n    step: 时间片编号（从 1 起）")
		.def("generate", &TimeStep::generate, py::arg("noises"), py::arg("arch_type"),
			"生成含噪调度。\n\n参数:\n"
			"    noises: 噪声模型，{OperationType: 概率} 字典（可为空）\n"
			"    arch_type: 架构常量，ARCH_QUBIT 或 ARCH_QUTRIT")
		.def("is_bad_branch", &TimeStep::is_bad_branch, py::arg("addr"),
			"判断地址 addr 是否落在坏分支区间（bad range）。")
		.def("get_bad_range_qubit", &TimeStep::get_bad_range_qubit, py::arg("bad_qubit"),
			"qubit 架构：由坏比特推导的坏分支地址区间 [lo, hi]。")
		.def("get_bad_range_qutrit", &TimeStep::get_bad_range_qutrit, py::arg("bad_qubit"),
			"qutrit 架构：由坏比特推导的坏分支地址区间 [lo, hi]。")
		.def("print", &TimeStep::print, "打印全部时间片的调度详情（fmt 输出到 stdout）。");

	// ---- qubit 架构 QRAMCircuit ----
	{
		using QC = qram_qubit::QRAMCircuit;
		py::class_<QC>(m, "QRAMCircuitQubit", R"doc(
qubit 架构 QRAM 装载电路的分支化稀疏仿真器。

以 (address, bus_input) 输入分支为轨迹载体，沿 TimeStep 生成的
含噪操作序列演化稀疏 SystemState，支持 good/bad 分支剪枝
（run_normal）、不剪枝基准（run_full）与保真度采样。

典型工作流:
    qram = QRAMCircuitQubit(addr_size, data_size[, memory])
    qram.set_noise_models({...})       # 可选
    qram.set_input_uniform(n)          # 或 set_input_random(n)
    qram.run_normal()                  # 或 run_full() / run("normal")
    qram.sample_and_get_fidelity()
)doc")
			.def(py::init<size_t, size_t>(), py::arg("addr_size"), py::arg("data_size"),
				"构造 QRAM 电路并分配 2^addr_size 条内存槽（未初始化）。")
			.def(py::init([](size_t a, size_t d, const memory_t& mem) { return QC(a, d, memory_t(mem)); }),
				py::arg("addr_size"), py::arg("data_size"), py::arg("memory"),
				"构造 QRAM 电路并以给定内存初始化（memory 长度须为 2^addr_size）。")
			.def_readonly("addr_size", &QC::addr_size, "地址位宽。")
			.def_readonly("data_size", &QC::data_size, "数据位宽。")
			.def("set_memory_random", &QC::set_memory_random,
				"以全局随机引擎随机填充整棵数据树。")
			.def("set_memory", (void (QC::*)(const memory_t&)) & QC::set_memory,
				py::arg("memory"), "设置数据树（memory 长度须为 2^addr_size）。")
			.def("get_memory", [](const QC& self) -> const memory_t& { return self.get_memory(); },
				py::return_value_policy::reference_internal, "返回数据树（引用视图）。")
			.def("get_qubit_num", &QC::get_qubit_num,
				"返回电路占用的物理比特数：2 * (2^addr_size - 1)。")
			.def("memory_size", &QC::memory_size, "返回内存槽总数。")
			.def("set_input_random", &QC::set_input_random, py::arg("n_inputs"),
				"随机采样 n 条输入分支，输入概率 ~ U(0,1) 后归一化。")
			.def("set_input_uniform", &QC::set_input_uniform, py::arg("n_inputs"),
				"随机采样 n 条输入分支，每条输入概率相等。")
			.def("set_noise_models", &QC::set_noise_models, py::arg("noises"),
				"设置噪声模型 {OperationType: 概率}。\n\n"
				"概率须在 [0,1] 内；Damping 要求 gamma < 1，越界抛出 RuntimeError。")
			.def("get_noise_models", [](const QC& self) -> const noise_t& { return self.noise_parameters; },
				py::return_value_policy::reference_internal, "返回当前噪声模型（引用视图）。")
			.def("is_noise_free", &QC::is_noise_free, "当前是否未设置任何噪声。")
			.def("has_damping", &QC::has_damping, "当前噪声模型是否包含 Damping。")
			.def("initialize_system", &QC::initialize_system,
				"由 TimeStep 生成含噪操作序列并复位全部分支组（run 系列内部调用）。")
			.def("run_normal", &QC::run_normal,
				"剪枝运行：坏分支全算 + 一条好分支作基准（日常使用的默认入口）。")
			.def("run_full", &QC::run_full,
				"不剪枝运行全部分支，作为剪枝结果的 ground truth 基准。")
			.def("run", (void (QC::*)()) & QC::run,
				"生成操作序列并按剪枝模式运行（等价于 run_normal 的内部入口）。")
			.def("run", (void (QC::*)(std::string)) & QC::run, py::arg("version"),
				"按版本字符串运行：\"full\"（不剪枝）/ \"normal\"（剪枝）/ \"fast\"（仅好分支）。")
			.def("sample_and_get_fidelity", &QC::sample_and_get_fidelity,
				"采样输出分支并计算装载保真度（理想态与实际态内积的模方）。")
			.def("get_fidelity", &QC::get_fidelity,
				"sample_and_get_fidelity 的别名。")
			.def("to_string", &QC::to_string, "返回电路概要信息。")
			.def("to_string_full_info", &QC::to_string_full_info,
				"返回含分支细节的完整信息。")
			.def("__str__", &QC::to_string);
	}

	// ---- qutrit 架构 QRAMCircuit ----
	{
		using QC = qram_qutrit::QRAMCircuit;
		py::class_<QC>(m, "QRAMCircuitQutrit", R"doc(
qutrit 架构 QRAM 装载电路的分支化稀疏仿真器。

每个树节点为 (addr ∈ {W, L, R}, data) 的三能级系统，以
SubBranch 轨迹沿含噪操作序列演化。工作流与 QRAMCircuitQubit
一致：构造 → 设内存 → 设噪声 → set_input_* → run_normal/run_full
→ sample_and_get_fidelity。
)doc")
			.def(py::init<size_t, size_t>(), py::arg("addr_size"), py::arg("data_size"),
				"构造 QRAM 电路并分配 2^addr_size 条内存槽（未初始化）。")
			.def(py::init<size_t, size_t, const memory_t&>(),
				py::arg("addr_size"), py::arg("data_size"), py::arg("memory"),
				"构造 QRAM 电路并以给定内存初始化。")
			.def_readonly("address_size", &QC::address_size, "地址位宽。")
			.def_readonly("data_size", &QC::data_size, "数据位宽。")
			.def("set_memory_random", &QC::set_memory_random,
				"以全局随机引擎随机填充整棵数据树。")
			.def("set_memory", (void (QC::*)(const memory_t&)) & QC::set_memory,
				py::arg("memory"), "设置数据树。")
			.def("get_memory", [](const QC& self) -> const memory_t& { return self.get_memory(); },
				py::return_value_policy::reference_internal, "返回数据树（引用视图）。")
			.def("get_qubit_num", &QC::get_qubit_num,
				"返回电路占用的物理比特数：2 * (2^address_size - 1)。")
			.def("memory_size", &QC::memory_size, "返回内存槽总数。")
			.def("set_input_random", &QC::set_input_random, py::arg("n_inputs"),
				"随机采样 n 条输入分支并填充分支概率。")
			.def("set_input_uniform", &QC::set_input_uniform, py::arg("n_inputs"),
				"随机采样 n 条输入分支，每条输入概率相等。")
			.def("set_noise_models", &QC::set_noise_models, py::arg("noises"),
				"设置噪声模型 {OperationType: 概率}。\n\n"
				"概率须在 [0,1] 内；Damping 要求 gamma < 1，越界抛出 RuntimeError。")
			.def("get_noise_models", [](const QC& self) -> const noise_t& { return self.noise_parameters; },
				py::return_value_policy::reference_internal, "返回当前噪声模型（引用视图）。")
			.def("is_noise_free", &QC::is_noise_free, "当前是否未设置任何噪声。")
			.def("has_damping", &QC::has_damping, "当前噪声模型是否包含 Damping。")
			.def("initialize_system", &QC::initialize_system,
				"由 TimeStep 生成含噪操作序列并初始化分支结构（run 系列内部调用）。")
			.def("run_normal", &QC::run_normal,
				"剪枝运行：坏分支全算 + 一条好分支作基准（日常使用的默认入口）。")
			.def("run_full", &QC::run_full,
				"不剪枝运行全部分支，作为剪枝结果的 ground truth 基准。")
			.def("run", (void (QC::*)(const std::string&)) & QC::run, py::arg("version"),
				"按版本字符串运行（\"full\" / \"normal\" 等，同 QRAMCircuitQubit.run）。")
			.def("sample_and_get_fidelity", &QC::sample_and_get_fidelity,
				"采样输出分支并计算装载保真度。")
			.def("get_branch_probs",
				[](const QC& self) -> const std::vector<double>& { return self.get_branch_probs(); },
				py::return_value_policy::reference_internal,
				"返回全部分支的归一化概率（引用视图）。")
			.def("to_string", &QC::to_string, "返回电路概要信息。")
			.def("to_string_full_info", &QC::to_string_full_info,
				"返回含分支细节的完整信息。")
			.def("__str__", &QC::to_string);
	}

	// ---- QRAMFullAmp：全振幅态向量桥接器 ----
	{
		py::class_<QRAMFullAmp>(m, "QRAMFullAmp", R"doc(
QRAM 电路到全振幅态向量的桥接器（仅适配 qutrit 架构）。

外部全振幅模拟器（或用户脚本）提供态向量与比特映射，
apply 将一次 QRAM 装载复合到该态向量上并返回新向量，
从而把 QRAM 仿真嵌入更大的电路演化流程。

示例:

    manipulator = QRAMFullAmp(2, 2, [0, 1, 2, 3])
    manipulator.set_noise_models({OperationType.Depolarizing: 1e-3})
    out = manipulator.apply(state, [0, 1], [2, 3], [], version="normal")
)doc")
			.def(py::init<size_t, size_t, const memory_t&>(),
				py::arg("addr_size"), py::arg("data_size"), py::arg("memory"),
				"构造桥接器：内部创建 qutrit QRAMCircuit 并载入给定内存。")
			.def("set_noise_models",
				[](QRAMFullAmp& self, const noise_t& noises) { self->set_noise_models(noises); },
				py::arg("noises"),
				"设置内部 QRAMCircuit 的噪声模型（约束同 QRAMCircuitQutrit）。")
			.def("apply", &QRAMFullAmp::apply,
				py::arg("state"), py::arg("address_qubits"), py::arg("data_qubits"),
				py::arg("other_qubits"), py::arg("version") = "normal",
				"把一次 QRAM 装载复合到全振幅态向量上。\n\n"
				"参数:\n"
				"    state: 复数振幅列表（长度 2^总比特数）\n"
				"    address_qubits / data_qubits / other_qubits: 三组比特的编号\n"
				"    version: \"normal\"（剪枝）或 \"full\"（不剪枝）\n"
				"返回:\n"
				"    装载后的新态向量（复数列表）");
	}
}
