动态算子扩展
============

.. contents:: 目录
   :local:
   :class: this-will-duplicate-information-and-it-is-still-useful-here

概述
----

动态算子扩展功能允许您在运行时编写自定义 C++ 量子算子，编译为共享库，并通过 Python 直接使用。这使得以下场景成为可能：

- **性能优化**：C++ 编译的算子比纯 Python 实现更快
- **自定义量子门**：实现标准库中不存在的专用量子门
- **快速原型开发**：无需重新编译整个 PySparQ 库即可测试新算子
- **算法专用优化**：为特定算法创建定制化的算子

适用场景
~~~~~~~~

动态算子最适合以下场景：

1. **性能关键操作**：需要 C++ 级别性能的复杂量子操作
2. **研究原型**：快速测试新的量子门设计
3. **算法定制**：为特定量子算法创建专用算子
4. **教学演示**：展示量子门的具体实现

前置条件
~~~~~~~~

使用动态算子前，请确保：

- 已安装 g++ 或 clang++ 编译器
- PySparQ 已正确安装（``pip install pysparq``）
- 了解基本的 C++ 和量子计算概念

架构概述
--------

动态算子的工作流程如下：

.. code-block:: text

   Python 层                         C++ 层
   +-------------------+              +-------------------+
   | compile_operator  | -----------> | g++ 编译          |
   +-------------------+              +-------------------+
            |                                  |
            v                                  v
   +-------------------+              +-------------------+
   | DynamicOpClass    | <----------> | .so 共享库        |
   | (type() 动态创建) |   ctypes     | (工厂函数)        |
   +-------------------+              +-------------------+
            |                                  |
            v                                  v
   +-------------------+              +-------------------+
   | SparseState       | <----------> | BaseOperator      |
   | (Python 对象)     |              | (C++ 实例)        |
   +-------------------+              +-------------------+

工厂函数
~~~~~~~~

编译生成的共享库导出以下 C 风格工厂函数：

.. code-block:: cpp

   // 创建算子实例
   extern "C" BaseOperator* create_operator(...);

   // 销毁算子实例
   extern "C" void destroy_operator(BaseOperator* op);

   // 获取算子名称
   extern "C" const char* get_operator_name();

   // 获取基类名称
   extern "C" const char* get_base_class();

   // Python 调用辅助函数
   extern "C" void apply_operator(BaseOperator* op, SparseState* state);
   extern "C" void apply_operator_dag(BaseOperator* op, SparseState* state);

基类选择
--------

动态算子支持两种基类：``BaseOperator`` 和 ``SelfAdjointOperator``。

对比
~~~~

.. list-table:: 基类对比
   :header-rows: 1

   * - 特性
     - SelfAdjointOperator
     - BaseOperator
   * - Dagger 行为
     - 自动等于自身
     - 需手动实现
   * - 适用场景
     - 厄米算子（X、Z、Hadamard）
     - 非厄米算子（相位旋转、一般酉门）
   * - 实现复杂度
     - 简单（只需 operator()）
     - 较复杂（需实现 dag()）
   * - 典型示例
     - Pauli 门、受控非门
     - 相位门、受控相位门

SelfAdjointOperator
~~~~~~~~~~~~~~~~~~~

当算子满足以下条件时使用 ``SelfAdjointOperator``：

- 算子是厄米的（等于其共轭转置）
- Dagger 操作应该等于自身（如 X 门）
- 需要更简单的实现

**示例：翻转算子**

.. code-block:: python

   cpp_code = """
   class FlipOp : public SelfAdjointOperator {
       size_t reg_id;
   public:
       FlipOp(size_t r) : reg_id(r) {}
       void operator()(std::vector<System>& state) const override {
           for (auto& s : state) {
               s.get(reg_id).value ^= 1;  // 翻转比特
           }
       }
   };
   """

BaseOperator
~~~~~~~~~~~~

当算子满足以下条件时使用 ``BaseOperator``：

- 算子不是厄米的
- 需要自定义 dagger 行为
- 相位旋转或一般酉门

**示例：相位旋转门**

.. code-block:: python

   cpp_code = """
   class PhaseGate : public BaseOperator {
       size_t reg_id;
       double phase;
   public:
       PhaseGate(size_t r, double p) : reg_id(r), phase(p) {}
       void operator()(std::vector<System>& state) const override {
           for (auto& s : state) {
               s.amplitude *= std::exp(std::complex<double>(0, phase));
           }
       }
       void dag(std::vector<System>& state) const override {
           for (auto& s : state) {
               s.amplitude *= std::exp(std::complex<double>(0, -phase));
           }
       }
   };
   """

基本用法
--------

步骤 1：编写 C++ 算子代码
~~~~~~~~~~~~~~~~~~~~~~~~~

编写继承自 ``BaseOperator`` 或 ``SelfAdjointOperator`` 的 C++ 类：

.. code-block:: python

   cpp_code = """
   class MyFlipOp : public SelfAdjointOperator {
       size_t reg_id;
   public:
       MyFlipOp(size_t r) : reg_id(r) {}
       void operator()(std::vector<System>& state) const override {
           for (auto& s : state) {
               s.get(reg_id).value ^= 1;
           }
       }
   };
   """

步骤 2：编译算子
~~~~~~~~~~~~~~~~

使用 ``compile_operator()`` 编译并创建 Python 类：

.. code-block:: python

   from pysparq.dynamic_operator import compile_operator

   MyFlipOp = compile_operator(
       name="MyFlipOp",
       cpp_code=cpp_code,
       base_class="SelfAdjointOperator",
       constructor_args=[("size_t", "reg_id")]
   )

步骤 3：使用算子
~~~~~~~~~~~~~~~~

像使用原生 PySparQ 算子一样使用动态算子：

.. code-block:: python

   import pysparq as ps

   ps.System.clear()
   state = ps.SparseState()
   q = ps.System.add_register("q", ps.Boolean, 1)

   op = MyFlipOp(reg_id=q)
   op(state)  # 应用算子

量子计算示例
------------

示例 1：受控相位门
~~~~~~~~~~~~~~~~~~~~

实现一个受控相位门，当控制位和目标位都为 ``|1⟩`` 时应用相位：

.. code-block:: python

   import pysparq as ps
   from pysparq.dynamic_operator import compile_operator
   import math

   # 定义受控相位门
   controlled_phase_code = """
   class ControlledPhase : public BaseOperator {
       size_t control_reg;
       size_t target_reg;
       double phase;
   public:
       ControlledPhase(size_t c, size_t t, double theta)
           : control_reg(c), target_reg(t), phase(theta) {}

       void operator()(std::vector<System>& state) const override {
           for (auto& s : state) {
               if (s.get(control_reg).value && s.get(target_reg).value) {
                   s.amplitude *= std::exp(std::complex<double>(0, phase));
               }
           }
       }

       void dag(std::vector<System>& state) const override {
           for (auto& s : state) {
               if (s.get(control_reg).value && s.get(target_reg).value) {
                   s.amplitude *= std::exp(std::complex<double>(0, -phase));
               }
           }
       }
   };
   """

   ControlledPhase = compile_operator(
       name="ControlledPhase",
       cpp_code=controlled_phase_code,
       base_class="BaseOperator",
       constructor_args=[
           ("size_t", "control_reg"),
           ("size_t", "target_reg"),
           ("double", "phase")
       ]
   )

   # 创建量子态
   ps.System.clear()
   state = ps.SparseState()
   c = ps.System.add_register("control", ps.Boolean, 1)
   t = ps.System.add_register("target", ps.Boolean, 1)

   # 初始化为 |11⟩
   ps.Init_Unsafe("control", 1)(state)
   ps.Init_Unsafe("target", 1)(state)

   # 应用受控相位门（π/4 相位）
   op = ControlledPhase(control_reg=c, target_reg=t, phase=math.pi/4)
   op(state)

   # 查看结果
   ps.pprint(state)

示例 2：量子游走算子
~~~~~~~~~~~~~~~~~~~~

实现量子游走的移动算子：

.. code-block:: python

   # 量子游走移动算子
   quantum_walk_code = """
   class QuantumWalkStep : public SelfAdjointOperator {
       size_t position_reg;
       size_t coin_reg;
       size_t n_positions;
   public:
       QuantumWalkStep(size_t pos, size_t coin, size_t n)
           : position_reg(pos), coin_reg(coin), n_positions(n) {}

       void operator()(std::vector<System>& state) const override {
           for (auto& s : state) {
               size_t coin_val = s.get(coin_reg).value;
               size_t pos = s.get(position_reg).value;

               // 硬币为 0 向右移动，为 1 向左移动
               if (coin_val == 0 && pos < n_positions - 1) {
                   s.get(position_reg).value = pos + 1;
               } else if (coin_val == 1 && pos > 0) {
                   s.get(position_reg).value = pos - 1;
               }
           }
       }
   };
   """

   QuantumWalkStep = compile_operator(
       name="QuantumWalkStep",
       cpp_code=quantum_walk_code,
       base_class="SelfAdjointOperator",
       constructor_args=[
           ("size_t", "position_reg"),
           ("size_t", "coin_reg"),
           ("size_t", "n_positions")
       ]
   )

示例 3：Grover 搜索 Oracle
~~~~~~~~~~~~~~~~~~~~~~~~~~

实现一个自定义的 Grover 搜索 Oracle：

.. code-block:: python

   # 自定义标记 Oracle
   oracle_code = """
   class MarkOracle : public SelfAdjointOperator {
       size_t data_reg;
       uint64_t target_value;
   public:
       MarkOracle(size_t d, uint64_t t) : data_reg(d), target_value(t) {}

       void operator()(std::vector<System>& state) const override {
           for (auto& s : state) {
               if (s.get(data_reg).value == target_value) {
                   s.amplitude *= -1.0;  // 标记目标状态
               }
           }
       }
   };
   """

   MarkOracle = compile_operator(
       name="MarkOracle",
       cpp_code=oracle_code,
       base_class="SelfAdjointOperator",
       constructor_args=[
           ("size_t", "data_reg"),
           ("uint64_t", "target_value")
       ]
   )

示例 4：哈密顿量演化
~~~~~~~~~~~~~~~~~~~~

实现时间演化算子：

.. code-block:: python

   # 哈密顿量演化算子
   hamiltonian_code = """
   class HamiltonianEvolution : public BaseOperator {
       size_t reg_id;
       double coupling_strength;
       double time;
   public:
       HamiltonianEvolution(size_t r, double g, double t)
           : reg_id(r), coupling_strength(g), time(t) {}

       void operator()(std::vector<System>& state) const override {
           double phase = coupling_strength * time;
           for (auto& s : state) {
               double value_phase = phase * s.get(reg_id).value;
               s.amplitude *= std::exp(std::complex<double>(0, value_phase));
           }
       }

       void dag(std::vector<System>& state) const override {
           double phase = -coupling_strength * time;
           for (auto& s : state) {
               double value_phase = phase * s.get(reg_id).value;
               s.amplitude *= std::exp(std::complex<double>(0, value_phase));
           }
       }
   };
   """

   HamiltonianEvolution = compile_operator(
       name="HamiltonianEvolution",
       cpp_code=hamiltonian_code,
       base_class="BaseOperator",
       constructor_args=[
           ("size_t", "reg_id"),
           ("double", "coupling_strength"),
           ("double", "time")
       ]
   )

高级功能
--------

构造函数参数
~~~~~~~~~~~~

``constructor_args`` 支持以下类型：

.. list-table:: 支持的参数类型
   :header-rows: 1

   * - C++ 类型
     - Python 类型
     - 用途
   * - ``size_t``
     - ``int``
     - 寄存器 ID、大小
   * - ``int``, ``long``
     - ``int``
     - 整数参数
   * - ``double``, ``float``
     - ``float``
     - 角度、相位、常数
   * - ``bool``
     - ``bool``
     - 开关标志
   * - ``uint64_t``
     - ``int``
     - 大整数

多参数示例：

.. code-block:: python

   constructor_args=[
       ("size_t", "control_reg"),
       ("size_t", "target_reg"),
       ("double", "angle"),
       ("int", "iterations")
   ]

编译缓存
~~~~~~~~

动态算子使用基于代码哈希的缓存机制，避免重复编译：

.. code-block:: python

   from pysparq.dynamic_operator import get_cache_info, clear_cache

   # 查看缓存状态
   info = get_cache_info()
   print(f"缓存目录: {info['cache_dir']}")
   print(f"缓存文件数: {info['so_count']}")
   print(f"缓存大小: {info['total_size_mb']} MB")

   # 清除缓存
   cleared_count = clear_cache()
   print(f"已清除 {cleared_count} 个缓存文件")

性能优化
~~~~~~~~

提高算子性能的技巧：

1. **选择正确的基类**：对于厄米算子，使用 ``SelfAdjointOperator`` 可简化实现
2. **减少内存分配**：避免在 ``operator()`` 中进行动态内存分配
3. **使用常量引用**：尽可能使用 ``const`` 引用
4. **避免复杂计算**：将复杂计算移到构造函数中预计算

故障排除
--------

常见编译错误
~~~~~~~~~~~~

**头文件找不到**

.. code-block:: text

   error: 'basic_components.h' file not found

解决方案：确保 PySparQ 已正确安装，头文件位于 include/ 目录中。

**未定义符号**

.. code-block:: text

   error: undefined reference to 'qram_simulator::System::get'

解决方案：检查命名空间是否正确（使用 ``qram_simulator``），确保类型匹配。

**Windows ABI 兼容性问题**

解决方案：Windows 上使用 MSVC 编译主库时，动态算子可能存在 ABI 兼容性问题。建议使用一致的编译器。

库加载错误
~~~~~~~~~~

**库文件不存在**

.. code-block:: python

   DynamicOperatorLoadError: Dynamic library does not exist: /path/to/lib.so

解决方案：检查编译是否成功，验证缓存目录权限。

**工厂函数找不到**

.. code-block:: python

   DynamicOperatorLoadError: Cannot find required factory functions

解决方案：确保代码中定义了 ``extern "C"`` 导出的工厂函数。

参数错误
~~~~~~~~

**无效的基类**

.. code-block:: python

   ValueError: base_class 必须是 ['BaseOperator', 'SelfAdjointOperator'] 之一

解决方案：确保 ``base_class`` 参数值为 ``"BaseOperator"`` 或 ``"SelfAdjointOperator"``。

**空名称或代码**

.. code-block:: python

   ValueError: name 必须是有效的字符串

解决方案：确保 ``name`` 和 ``cpp_code`` 参数不为空。

API 参考
--------

详细 API 文档请参考 :doc:`../api/dynamic_operator`。
