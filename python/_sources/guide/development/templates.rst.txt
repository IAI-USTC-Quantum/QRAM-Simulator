代码模板
========

寄存器类型
----------

.. code-block:: cpp

   UnsignedInteger  // 无符号整数
   SignedInteger     // 有符号整数
   Boolean          // 单比特
   Rational         // 有理数（用于角度计算）

C++ 开发模板
------------

.. code-block:: cpp

   #include "sparse_state_simulator.h"
   #include "system_operations.h"
   #include "quantum_arithmetic.h"

   using namespace qram_simulator;

   // 1. 在 System 中声明寄存器
   auto addr_reg = System::add_register("addr", UnsignedInteger, addr_size);
   auto data_reg = System::add_register("data", UnsignedInteger, data_size);

   // 2. 创建稀疏态（自动初始化为 |0...0⟩）
   std::vector<System> state;
   state.emplace_back();

   // 3. 应用量子操作
   Hadamard_Int(addr_reg)(state);           // 叠加态
   QRAMLoad(qram, addr_reg, data_reg)(state); // QRAM 加载
   Add_UInt_UInt(data_reg, result_reg)(state); // 算术运算

   // 4. 测量/输出
   StatePrint(Detail)(state);

Python 开发模板
---------------

.. code-block:: python

   import pysparq as ps

   # 1. 清理静态状态
   ps.System.clear()

   # 2. 声明寄存器
   ps.System.add_register("addr", ps.UnsignedInteger, 4)
   ps.System.add_register("data", ps.UnsignedInteger, 4)

   # 3. 创建稀疏态
   state = ps.SparseState()

   # 4. 应用操作
   ps.Hadamard_Int("addr")(state)
   ps.QRAMLoad(qram, "addr", "data")(state)

   # 5. 读取结果
   ps.pprint(state)

Grover 搜索模板
---------------

.. code-block:: cpp

   // 创建 QRAM
   qram_qutrit::QRAMCircuit qram(addr_size, data_size);
   qram.set_memory_random();

   // 声明寄存器
   auto addr_reg = System::add_register("addr", UnsignedInteger, addr_size);
   auto data_reg = System::add_register("data", UnsignedInteger, data_size);
   auto target_reg = System::add_register("target", UnsignedInteger, data_size);

   // 创建态
   std::vector<System> state;
   state.emplace_back();
   Init_Unsafe("target", search_target)(state);

   // 应用 Grover 算子
   for (size_t i = 0; i < n_repeats; ++i) {
       auto temp_reg = AddRegister("temp", UnsignedInteger, data_size)(state);
       GroverOperator(&qram, addr_reg, temp_reg, target_reg)(state);
       RemoveRegister(temp_reg)(state);
   }

块编码模板
----------

参考 ``SparQ_Algorithm/include/block_encoding.h`` 实现酉矩阵的块编码。

哈密顿量模拟
------------

参考 ``Experiments/CKS/`` 中的 CKS（Carnegie-Kellam-Schulten）算法实现。
