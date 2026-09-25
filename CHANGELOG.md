# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

---

## [Unreleased]

### Added
- **pybind11 绑定层回归本仓库**：`bindings/python/`（`core_binding.cpp` +
  `qram_simulator/__init__.py` + pytest 套件）导出核心工作类——
  `QRAMCircuitQubit` / `QRAMCircuitQutrit` / `QRAMFullAmp` / `TimeStep`
  （含 `TimeSlices` / `OperationPack` / `Operation` 轻量视图）、
  `OperationType` 枚举、`ARCH_QUBIT` / `ARCH_QUTRIT` 常量与全局随机种子
  控制（`set_seed` / `get_seed`），供外部 Python 库与脚本化实验直接驱动
  完整的"构造 → 设噪声 → 运行 → 保真度"工作流
- **根 `pyproject.toml`**：scikit-build-core + pybind11 + setuptools-scm
  打包 `qram-simulator`（cp310–313，manylinux x86_64 + win_amd64），
  `__version__` 运行时从 dist-info 读取（规避 scikit-build-core 按
  .gitignore 过滤 wheel 文件的坑）
- **`.github/workflows/pypi-publish.yml`**：tag `v*` / GitHub Release 触发
  → cibuildwheel 多平台 wheel + sdist（含自包含校验）→ PyPI trusted
  publishing（OIDC）；`python-bindings.yml` 守护 CI（wheel 构建安装 +
  pytest，ubuntu/windows × py3.10/3.12）
- **Sphinx 文档站点**（`docs/sphinx/`）：单一站点 = MyST 指南（安装/
  快速上手/架构重写）+ breathe 吸入 Doxygen C++ API + pybind11-stubgen →
  autoapi 的 Python API + 论文文档迁移；`docs.yml` 重写为该流水线，
  gh-pages 全量替换为 Sphinx 站点（Doxyfile 开启 `GENERATE_XML`）
- **全仓中文 Doxygen 注释**：`QRAM/include/` 五个头文件与
  `Common/include/` 全部公有头（QRAMCircuit ×2、TimeStep、State/Branch
  系列、QRAMFullAmp、矩阵、随机引擎、日志等）补齐 `/** @brief @param
  @return */` 注释，注释与 Doxygen/breathe/Sphinx 链路打通
- **`LICENSE`**：补齐 Apache-2.0 全文（pyproject 与 README 此前已声明该许可，
  GitHub 许可检测由此生效）；CONTRIBUTING 克隆示例统一为 GitHub 地址

### Changed
- **第二轮分仓:SparQ 框架整体迁出,本仓库收敛为纯 C++ QRAM 基座**。
  `SparQ/`、`SparQ_Algorithm/`、`bindings/python/`（薄绑定）、`examples/`、
  算法系实验（QDA/Grover/StatePreparation/QCNN/QFT/CKS/Shor/GHZ/
  ErrorFiltration/GPUTime）与依赖 SparQ 算子的 QRAMFidelity(v1)/QRAM_Qubit
  全部迁至 SparQSim 仓库;CommonTest 按归属拆分（算法块随完整版留在
  SparQSim,本仓库保留 Common 与纯 QRAM 部分）。依赖方向固化为
  **SparQSim → QRAM-Simulator**;本仓库不再包含任何 SparQ 代码与 Python 组件,
  `qram-simulator` 包改由 SparQSim 仓库构建发布
  （注:该决定后被本次 Unreleased 的绑定层回归条目取代——`qram-simulator`
  改由本仓库独立构建发布）
- 伞形 `SparQ` CMake 目标随 SparQ_Algorithm 迁出（现定义于 SparQSim 根
  CMakeLists）;本仓库导出目标收敛为 `SparQ_Common` + `SparQ_QRAMSimulator`
  （平铺头文件 BUILD_INTERFACE 随目标暴露,供 SparQSim 组合）,
  `SparQ_Common` 补 PUBLIC 链接 fmt
- 构建开关收敛为 `QRAM_BUILD_TESTS` / `QRAM_BUILD_EXPERIMENTS`
  （`QRAM_BUILD_PYTHON_BINDINGS` 与 `BUILD_EXAMPLES` 移除）;
  consumer-mode CI 改 `--target SparQ_QRAMSimulator`
- **第一轮拆分**:本仓库（QRAM-Simulator monorepo）拆分为两个独立仓库,
  本仓库名保留 QRAM-Simulator,转为纯 C++ 核心仓库独立发版;
  PySparQ/pysparq 全功能 Python 框架迁移至 **SparQSim** 仓库,以 git
  submodule(相对 URL `../QRAM-Simulator.git`)引用并编译本仓库核心。
  本条目之前的 PySparQ 相关历史条目见 SparQSim 仓库 CHANGELOG 及本文件
  git 历史(路径已随拆分移除,`git log --follow` 可追溯)
- 根 CMakeLists 新增 `QRAM_BUILD_TESTS` / `QRAM_BUILD_EXPERIMENTS` /
  `QRAM_BUILD_PYTHON_BINDINGS` 开关,供 SparQSim 以 add_subdirectory
  方式消费(测试/实验默认 ON,绑定默认 OFF)

### Removed
- `PySparQ/`、旧 `pyproject.toml`、`.cibuildwheel-hooks/`、`docs/sphinx/`、
  Python 示例与各 workflow 中的 Python 作业(全部迁往 SparQSim)

### Added
- **qram_simulator 薄 Python 绑定**(`bindings/python/`):刻意最小化的
  核心原语 API——System/SparseState 寄存器管理、Init/Hadamard/X、
  Add 算术族、QFT、MeasureZ/Reset/Probability、PartialTrace、
  QRAMCircuit_qutrit/QRAMLoad、StatePrint;含 pytest 冒烟测试
- 根 `pyproject.toml`(qram-simulator 包):scikit-build-core + setuptools-scm,
  wheel 构建经 cmake.define 关闭 tests/experiments,sdist 保持自包含
- **宽度与截断约定**(`docs/operators.md` 新章,权威契约):LSB 对齐;读扩展由
  名字槽位承载(`_UInt_` 零扩展 / `_SInt_` 符号扩展 / `AnyInt` 按寄存器声明
  类型),release 构建同样成立;结果 `mod 2^out_width` XOR 写入,输出宽度可与
  输入任意不同;域外全量化(除零→商 0);flag 谓词在全精度域求值;宽度
  1..64 全支持;唯一行为变更是 `Add_AnyInt_AnyInt_InPlace` 的 AnyInt 槽语义
- **16 个新算术算子**(CPU+CUDA+PySparQ 绑定,整数核+flag 谓词族,服务
  pyqecclang QFVM 算术自动编译的去 compile_operator 化):
  `Sub_UInt_UInt`、`Neg_UInt`、`Abs_SInt`、`Mul_UInt_UInt`、`Div_UInt_UInt`、
  `Sqrt_UInt`、`Select_Bool_UInt_UInt`、`And/Or/Xor_UInt_UInt`、
  `Less_SInt_SInt`、`Carry_UInt_UInt`、`Overflow_SInt_SInt`、
  `MulOverflow_UInt_UInt`、`IsZero_UInt`、`Negative_SInt`
- **Common/include/basic.h**:`width_mask` / `isqrt_u64`(HOST_DEVICE,
  整数位对算法,CPU/CPU-CUDA 位一致)
- **PySparQ/pysparq/conformance.py**:`two_complement_decode` / `sign_extend`
  / `WIDTH_BOUNDARIES` / 声明式 `width_matrix_case`(宽度组合 × 独立模型 ×
  非零输出起点碰撞 × 双序 dagger × 叠加线性 × 控制矩阵一站式 runner);
  **PySparQ/test/test_width_conventions.py**:16 新算子 + 存量 Add/Assign/
  Compare 的宽度边界矩阵(含 63/64 位边界)
- **SparQ/test/quantum_arithmetic.cpp**:16 新算子混合宽度真值表 + 参数化
  宽度扫描 + Add_AnyInt 新语义专项;**test/GPUTest/ArithmeticTest**:CUDA
  修复项的一致性用例(新)
- **docs/naming_conventions.md**:槽位语义承载规则与谓词算子族命名规则增补

### Changed
- **CUDA 可逆性/UB 修复**:`Add_UInt_UInt` 无条件路径由覆盖写改为 XOR+宽度
  掩码(此前对非零输出寄存器不可逆);`Add_UInt_UInt_InPlace`/`Add_Mult`/
  `Assign` CUDA 路径补宽度掩码(消除 CPU/GPU 不等宽行为分歧);全部
  `%=(1<<w)`/`1<<w` 于 w=64 的 UB 改为 `width_mask`;`cu_as_uint64/double/
  bool/int64` 与 `CuConditionSatisfied` 的 `1ULL<<64` UB 修复(64 位寄存器
  CUDA 读取此前掩码为 0)
- **`Add_AnyInt_AnyInt_InPlace` 语义迁移(唯一行为变更)**:AnyInt 槽按寄存器
  声明类型扩展(SInt 操作数此前按无符号位模式读);宽度/类型改执行期读取;
  补 `lhs==rhs` 别名拒绝(always-on)。等宽非负使用不受影响
- **`CustomArithmetic`**:输出 XOR 补宽度掩码(此前裸写可越宽)
- **`consumer_runtime_inventory.json`**:accepted 清单增补 16 个新算子
  (xor_into);契约测试增补 Sub/Div/Select 独立模型用例
- **docs/naming_conventions.md** (naming-refactor task): authoritative operator
  naming ruleset — symbol layers, the `Family_Slots_Variants` grammar with closed type-tag /
  modifier / variant vocabularies, slot and `_Bool` semantics, the `_InPlace`
  naming contract, dagger-first inversion principle, gate-family rules
  (`<Axis>_Bool` without the `gate` infix), RPN composition reading, the
  deprecation/alias mechanism, and the full rename audit table
- **SparQ/src/qft.cpp**, **SparQ/include/qft.h**: `QFT::dag()` now applies
  the inverse transform (delegating to `InverseQFT` with control conditions
  preserved) and is exposed in PySparQ via `BIND_DAG_METHODS(QFT)`
- **PySparQ/pysparq/__init__.py**: PEP 562 module `__getattr__` providing
  old spellings of the 18 renamed Python operator names as deprecated aliases
  (`DeprecationWarning`), keeping external consumers (pyqecclang, qfvm)
  working during migration; `PySparQ/consumer_runtime_inventory.json` gains a
  `deprecated_aliases` section documenting the mapping
- **PySparQ/pysparq/rir.py**: native execution of QECC.Lang RIR JSON documents
  (schema versions 0.1–0.3) on PySparQ sparse states — module-graph expansion at
  interpretation time (`Call` inlining via register renaming, `Repeat` replay,
  `Adjoint` inversion, multi-bit `Control` accumulation) with native operator
  dispatch (`Add_ConstUInt_InPlace`, `GlobalPhase`, `QRAMLoad`) whenever an
  RIR operand aligns with a whole register; exposed as `run_rir`, `run_rir_file`,
  `load_rir`, `RIRResult`, `RIRError`
- **PySparQ/test/test_rir.py**: regression tests for RIR loading, gate-level
  execution, module expansion, register-view arithmetic and QRAM loading
- **docs/sphinx/source/guide/rir.rst**, **docs/sphinx/source/api/rir.rst**,
  **docs/pysparq.md**: documentation of PySparQ as the natural RIR interpreter,
  with worked examples (Bell state, `Call`/`Repeat`/`Adjoint` module graphs,
  register views, QRAM loading)
- **SparQ/include/measurement.h**, **SparQ/src/measurement.cpp**: First-class
  seedable sparse-state operations for a dynamic executor: `MeasureZ`
  (projective Z-basis measurement: Born-rule sampling + collapse +
  renormalize), `Reset` (measurement + classical conditioned flip to a
  target value), `Probability` (read-only Born-rule query + full
  single-register outcome distribution)
- **PySparQ/core.cpp**: pybind11 bindings for `MeasureZ`, `Reset`,
  `Probability`, and the global seedable RNG (`set_seed`, `get_seed`,
  `reseed`, `time_seed`)
- **PySparQ/pysparq/conformance.py**: Reusable semantic conformance harness
  (arbitrary nonzero outputs, basis-exhaustive/spot-sampled coverage,
  output-collision detection, superposition linearity, positive/negative/
  multi-register controls, forward+dagger and dagger+forward identity)
- **PySparQ/test/test_semantic_conformance.py**: Conformance matrix applied
  to representative built-ins (`Add_UInt_UInt`, `Add_UInt_UInt_InPlace`,
  `Assign`, `Compare_UInt_UInt`, `CustomArithmetic`, `Swap_General_General`,
  `Xgate_Bool`, `FlipBools`, `QRAMLoad`), plus a negative-control test
  proving the harness rejects a destructive overwrite/clearing-dagger
  implementation
- **PySparQ/test/test_measurement.py**: `MeasureZ`/`Reset`/`Probability`
  tests, including determinism under `set_seed` and Born-rule cross-checks
  against a dense-state reference
- **PySparQ/pysparq/conformance.py**: `assert_collision_free_for_output_starts`
  strengthens collision detection for `xor_into` operators by re-checking
  injectivity with the output register(s) seeded at several arbitrary
  (including nonzero) starting values, not only the conventional
  clean/zero-ancilla start; applied to `Add_UInt_UInt`, `Assign`,
  `Compare_UInt_UInt`, `CustomArithmetic`, and `QRAMLoad` in
  `test_semantic_conformance.py`, plus a negative-control test proving it
  rejects the destructive overwrite implementation at nonzero starts too
- **PySparQ/test/test_measurement.py**: `TestMeasureZNormalizationValidation`
  and `TestRegisterValidation` cover the C++ hardening below (non-normalized
  input rejection, unknown/out-of-range/inactive/duplicate register
  rejection, out-of-width-range Reset target/Probability value rejection)

### Fixed
- **SparQ/include/measurement.h**, **SparQ/src/measurement.cpp**:
  - `MeasureZ` now validates that the input state's total Born-rule
    probability is finite and within `kNormalizationThreshold` (`1e-5`,
    matching `CheckNormalization`'s default) of 1.0 before sampling, and
    raises instead of silently biasing towards/falling back to the last
    branch when a caller passes a non-normalized state. `Reset` inherits
    this guard since it delegates to `MeasureZ`.
  - `MeasureZ`/`Reset`/`Probability` constructors now validate every
    register name/id: unknown names and out-of-range or inactive
    (removed) ids raise `invalid_argument` instead of silently resolving
    to `SIZE_MAX`/an unchecked id (previously undefined behavior on use);
    duplicate registers within one call (e.g. `Reset(["a", "a"], [1, 2])`)
    are rejected instead of silently applying a contradictory target.
  - `Reset` targets and `Probability` values are validated against
    `System::size_of(id)` and now raise `invalid_argument` if they do not
    fit the register's bit width, instead of being silently truncated.

### Changed
- **Naming refactor (19 operator renames, per docs/naming_conventions.md)**:
  the gate family loses the `gate` infix (`Xgate_Bool` → `X_Bool`, 11 gates);
  `inverseQFT` → `InverseQFT` (last lowercase-initial class name); truthless
  type tags fixed (`GlobalPhase_Int` → `GlobalPhase`,
  `Div_Sqrt_Arccos_Int_Int` → `Div_Sqrt_Arccos_UInt_UInt`,
  `Sqrt_Div_Arccos_Int_Int` → `Sqrt_Div_Arccos_Int_UInt`);
  `AddAssign_AnyInt_AnyInt_InPlace` → `Add_AnyInt_AnyInt_InPlace`;
  `Hadamard_PartialQubit` → `Hadamard_Partial`;
  `CondRot_General_Bool_fast` → `CondRot_General_Bool_Fast` (C++ only);
  `QuantumBinarySearchFast` → `QuantumBinarySearch_Fast`. Old C++ spellings
  remain available as `[[deprecated]] using` aliases and old Python spellings
  via the deprecation aliases above; all in-repo consumers (SparQ,
  SparQ_Algorithm, Experiments, tests, examples, PySparQ, docs) use the new
  names. `GetRotateAngle_Int_Int` was verified truthful (signed
  interpretation) and unchanged
- **docs**: sphinx operator pages drift fixed (short names missing `_InPlace`
  for `Add_ConstUInt_InPlace`/`Add_Mult_UInt_ConstUInt_InPlace`/
  `Mod_Mult_UInt_ConstUInt_InPlace`/`Shift*_InPlace`; removed the phantom
  `CheckNormalization_Renormalize`; corrected the false
  "ShiftLeft.dag() not implemented" claim); CLAUDE.md no longer claims
  nonexistent CNOT/Toffoli classes; CONTRIBUTING.md's function-naming example
  corrected to the real `noise_free_impl` and now links the naming ruleset
- **SparQ/include/basic_components.h**, **SparQ/src/basic_components.cpp**:
  CPU basis-state register storage now uses `std::vector<StateStorage>`.
  `CACHED_REGISTER_SIZE` is the initial reserved block rather than a hard CPU
  limit; storage grows on demand and removed slots are reused. CUDA retains
  its fixed layout and 64-register ceiling required by device kernels and
  their `uint64_t` active-register bitmap.
- **PySparQ/pysparq/dynamic_operator/__init__.py**,
  **PySparQ/pysparq/dynamic_operator/README.md**: Documented that
  `compile_operator()` cannot prove unitarity and is forbidden on the
  supported QCFD path (QECC.Lang-driven qfvm/qnls/qham); QCFD semantics
  must use named PySparQ built-ins validated by `pysparq.conformance`
- **README.md**: Documented the new seedable measurement/reset/probability
  API in the Register Level key-API section

---

## [0.1.1] - 2026-05-01

### Added
- **PySparQ/core.cpp**: Native C++ bindings for CKS primitives:
  `CondRot_General_Bool_QW`, `QuantumBinarySearchFast`, `GetRowAddr`,
  `GetDataAddr`; `SparseMatrix` named constructor arguments and
  read-only properties (`nnz_col`, `n_row`, `data_size`, `sparsity`)
- **PySparQ/pysparq/__init__.py**: Export 4 new CKS primitives
- **PySparQ/pysparq/algorithms/cks_solver.py**: Full refactor:
  `_CKSWalkEnvironment` class with bytecode-based unpacking, global caches
  (`_CKS_ENV_CACHE`, `_CKS_STATE_STEP_CACHE`), `SparseMatrix.from_dense()`
  with L2-normalization, `TOperator` oracle call order aligned to C++,
  `QuantumWalkNSteps` power-of-two padding
- **PySparQ/pysparq/algorithms/qda_solver.py**: `BlockEncodingHs.dag()`
  implemented (21-operation reverse sequence); `qda_solve()` rewritten
  with correct `n_bits` and step formula
- **PySparQ/test/algorithms/test_cks_integration.py**: Full end-to-end
  quantum walk execution against C++ reference matrices
- **PySparQ/test/algorithms/test_qda_integration.py**: `WalkS` fidelity
  tests against C++ `CorrectnessTest_QDA_CompareList.inl` reference values

### Changed
- **PySparQ/src/qda_algo.cpp**: Conditional rotation primitives refactored
  to `CondRot_Fixed_Bool` and `CondRot_General_Bool_fast`; old
  function-based API removed
- **PySparQ/**: `block_encoding.py`, `state_preparation.py`,
  `dynamic_operator/compiler.py`, `dynamic_operator/operator_wrapper.py`
  updated to new condrot API
- **SparQ/src/condrot.cpp**: Internal condrot implementation aligned
- **SparQ_Algorithm/**: `hamiltonian_simulation.h/cpp` aligned with new
  condrot API
- **docs/paper/reproduction.md**: Updated CUDA build instructions;
  upstream repo URL fixed
- **CONTRIBUTING.md**: Updated clone instructions to upstream;
  added `uv venv` guidance
- **CLAUDE.md**: Added CKS/QDA Python porting status and open issues

### Fixed
- **PySparQ/src/qda_algo.cpp**: Trailing newline added

### Removed
- **PySparQ/**: Old function-based conditional rotation Python API
  hidden from `__init__.py`

### Documentation
- **docs/paper/reproduction.md**: Fix repository URL from fork to upstream
- **CONTRIBUTING.md**: Fix clone URL from fork to upstream
- **CLAUDE.md**: Add CKS/QDA Python porting status and known open issues

---

## [0.1.0] - 2025-04-15

### Added

#### 核心模拟器
- **稀疏态量子模拟器核心（SparQ）**
  - 基于稀疏向量表示的量子态存储
  - 高效的门操作执行引擎
  - 支持任意数量的量子比特
  - 内置归一化验证和状态检查

#### QRAM 实现
- **Qutrit-based QRAM 电路**
  - 层次化的树形结构表示
  - 支持可配置的地址线和数据线位数
  - 时步逻辑生成量子操作序列
  - 分支概率追踪和采样
  
- **Qubit-based QRAM 电路**
  - 硬件兼容的实现方式
  - 与 qutrit 版本共享相同 API

- **CUDA GPU 加速支持**
  - GPU 内存管理（`thrust::device_vector`）
  - 并行门操作内核
  - 异步执行支持
  - CPU-GPU 混合执行策略

#### 基础量子门
- **单比特门**：H（Hadamard）、X、Y、Z、S、T
- **旋转门**：RX、RY、RZ、通用旋转门
- **多比特门**：CNOT、Toffoli、多控制门
- **条件门**：条件旋转、条件交换
- **相位门**：S、T、相位旋转、并行相位操作

#### 高级量子算法
- **量子傅里叶变换（QFT）**
- **Grover 搜索算法**
- **量子算术运算**
  - 量子加法器
  - 量子乘法器
  - 量子比较器
- **块编码（Block Encoding）**
- **态制备（State Preparation）**
- **量子随机漫步**
- **哈密顿量模拟**

#### 噪声模型
- **去极化噪声（Depolarizing）**
- **振幅阻尼噪声（Amplitude Damping）**
- **噪声参数可配置**
- **噪声影响分析工具**

#### Python 绑定（PySparQ）
- **完整的 Python API**
  - 所有核心类暴露到 Python
  - 类型提示支持（`_core.pyi`）
- **Pybind11 封装**
  - 高效的 C++-Python 互操作
  - 自动内存管理
- **pip 安装支持**
  - `pyproject.toml` 配置
  - 预编译 wheel 分发

#### 实验代码
- **QRAM 保真度测试**
  - `QRAMFidelityTest.cpp`：主模拟 + 性能分析
  - `QRAMSimulatorTest.cpp`：完整 vs 普通模拟器对比
- **误差过滤实验**
  - `testMultiEFQRAM.cpp`：多误差过滤 QRAM
- **算法验证**
  - QFT、Grover、Shor 算法
  - 量子卷积神经网络（QCNN）
  - 量子微分算法（QDA）
  - GHZ 态制备

#### 基础设施
- **CMake 构建系统**
  - 跨平台支持（Linux、Windows、macOS）
  - 自动依赖检测
  - CUDA 可选编译
- **Eigen 集成**
  - 头文件形式嵌入（ThirdParty/eigen-3.4.0）
  - 稀疏/稠密矩阵运算支持
- **日志系统**
  - 分级日志（DEBUG、INFO、WARN、ERROR）
  - 性能计时支持
- **单元测试框架**
  - 核心模块测试覆盖
  - CI/CD 集成（GitHub Actions）

### Changed
- 无（初始版本）

### Deprecated
- 无

### Removed
- 无

### Fixed
- 无（初始版本）

### Security
- 无

---

## 版本历史概览

| 版本 | 日期 | 主要更新 |
|------|------|----------|
| 0.1.1 | 2026-05-01 | CKS/QDA Python primitive alignment, native C++ bindings, integration tests activated |
| 0.1.0 | 2025-04-15 | 首次发布，包含完整的 QRAM 模拟器、稀疏态模拟核心、Python 绑定和实验代码 |

---

## 未来计划

### [1.1.0] - 计划

#### 预期新增功能
- [ ] 更多噪声模型（相位阻尼、比特翻转等）
- [ ] 可视化工具（量子态演化图）
- [ ] 性能优化（向量化、更高效的稀疏操作）
- [ ] 更多算法实现（VQE、QAOA）
- [ ] 交互式 Jupyter Notebook 教程

#### 预期改进
- [ ] Python API 文档完善
- [ ] 更多使用示例
- [ ] 性能基准测试套件

---

## 参考

- [Keep a Changelog](https://keepachangelog.com/)
- [Semantic Versioning](https://semver.org/)
- 项目文档：[README.md](README.md) | [ARCHITECTURE.md](docs/architecture.md)
