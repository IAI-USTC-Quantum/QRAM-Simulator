示例
====

本节展示如何使用 PySparQ 从零构建量子算法——从初态创建、已有算子的使用，到自定义算子，最终搭建 Block Encoding 电路。

.. contents:: 目录
   :local:
   :class: this-will-duplicate-information-and-it-is-still-useful-here

前置要求
--------

PySparQ 已安装，且 C++ 核心已编译（GPU 支持可选）。

.. code-block:: bash

   pip install pysparq


示例 1：创建初态
----------------

量子电路的起点是量子态（``SparseState``）。在 PySparQ 中，量子比特以**寄存器**（Register）为单位组织，而不是单个 qubit。

创建系统并初始化寄存器：

.. code-block:: python

   import pysparq as ps

   # 清理系统（推荐在每次运行前调用）
   ps.System.clear()

   # 添加一个 4 比特的整数寄存器
   ps.System.add_register("q", ps.UnsignedInteger, 4)

   # 创建量子态
   state = ps.SparseState()
   print(state)
   # 输出：
   # StatePrint (mode=Detail)
   # |(0)q : UInt4 |
   # 1.000000+0.000000i  q=|0>

初始态为 :math:`|0000\rangle`。通过 Hadamard 变换创建叠加态：

.. code-block:: python

   # 对整个寄存器应用 Hadamard → 等价于对每个比特施 H
   ps.Hadamard_Int_Full("q")(state)

   # 打印状态（稀疏格式：只显示非零振幅）
   ps.print(state)
   # 输出：
   # StatePrint (mode=Detail)
   # |(0)q : UInt4 |
   # 0.250000+0.000000i  q=|0>
   # 0.250000+0.000000i  q=|1>
   # ...
   # 0.250000+0.000000i  q=|15>
   # 16 个等概率幅叠加态（每个 0.25）

将某一基态设为特定值：

.. code-block:: python

   ps.Init_Unsafe("q", 5)(state)   # 将寄存器初始化为 |0101⟩
   ps.print(state)
   # 输出：
   # StatePrint (mode=Detail)
   # |(0)q : UInt4 |
   # 0.250000+0.000000i  q=|5>   ← 16 个振幅都变为 |5⟩

通过加法算子驱动态演化（寄存器级编程的核心）：

.. code-block:: python

   ps.Add_ConstUInt("q", 1)(state)  # |q⟩ → |q+1⟩，所以 |5⟩ → |6⟩
   ps.print(state)
   # 输出：
   # StatePrint (mode=Detail)
   # |(0)q : UInt4 |
   # 0.250000+0.000000i  q=|6>
   # 0.250000+0.000000i  q=|6>
   # ...（16 个相同基态）


示例 2：使用已有算子
--------------------

PySparQ 提供丰富的内置算子，覆盖算术、QRAM、QFT、条件旋转等类别。以下展示如何组合使用。

加法、乘法、移位
~~~~~~~~~~~~~~~~

两个整数寄存器之间的算术运算：

.. code-block:: python

   ps.System.clear()
   ps.System.add_register("a", ps.UnsignedInteger, 4)
   ps.System.add_register("b", ps.UnsignedInteger, 4)

   state = ps.SparseState()
   ps.Init_Unsafe("a", 3)(state)   # a = 3
   ps.Init_Unsafe("b", 7)(state)   # b = 7

   # a ← a + b（out_reg = a，即 in-place）
   ps.Add_UInt_UInt("a", "b", "a")(state)
   ps.print(state)           # a = 10
   # 输出：
   # StatePrint (mode=Detail)
   # |(0)a : UInt4 | |(1)b : UInt4 |
   # 1.000000+0.000000i  a=|10> b=|7>

   # a ← a × 2
   ps.Mult_UInt_ConstUInt("a", 2)(state)
   ps.print(state)           # a = 20
   # 输出：
   # StatePrint (mode=Detail)
   # |(0)a : UInt4 | |(1)b : UInt4 |
   # 1.000000+0.000000i  a=|20> b=|7>

   # a ← a << 1（左移1位，等价于乘2）
   ps.ShiftLeft("a", 1)(state)
   ps.print(state)           # a = 40
   # 输出：
   # StatePrint (mode=Detail)
   # |(0)a : UInt4 | |(1)b : UInt4 |
   # 1.000000+0.000000i  a=|40> b=|7>


QRAM 数据加载
~~~~~~~~~~~~~~

将经典数组加载到量子态中，支持地址叠加态下的批量查询：

.. code-block:: python

   import numpy as np

   ps.System.clear()
   n_addr, n_data = 3, 4
   ps.System.add_register("addr", ps.UnsignedInteger, n_addr)
   ps.System.add_register("data", ps.UnsignedInteger, n_data)

   state = ps.SparseState()

   # 经典数据（8 个内存位置，每位置 4 比特）
   memory = np.array([1, 3, 5, 7, 2, 4, 6, 8], dtype=np.uint64)

   # 对地址寄存器叠加 → 同时查询所有地址
   ps.Hadamard_Int_Full("addr")(state)

   # QRAM 加载：data = memory[addr]
   qram = ps.QRAMCircuit_qutrit(n_addr, n_data, memory)
   ps.QRAMLoad(qram, "addr", "data")(state)

   # 状态包含所有 (addr, memory[addr]) 对的振幅
   ps.print(state)
   # 输出：
   # StatePrint (mode=Detail)
   # |(0)addr : UInt3 | |(1)data : UInt4 |
   # 0.353553+0.000000i  addr=|0> data=|1>
   # 0.353553+0.000000i  addr=|1> data=|3>
   # 0.353553+0.000000i  addr=|2> data=|5>
   # 0.353553+0.000000i  addr=|3> data=|7>
   # 0.353553+0.000000i  addr=|4> data=|2>
   # 0.353553+0.000000i  addr=|5> data=|4>
   # 0.353553+0.000000i  addr=|6> data=|6>
   # 0.353553+0.000000i  addr=|7> data=|8>
   # 8 个叠加态，每个振幅 1/√8 ≈ 0.354


QFT 与逆 QFT
~~~~~~~~~~~~

量子傅里叶变换及其逆变换：

.. code-block:: python

   ps.System.clear()
   ps.System.add_register("reg", ps.UnsignedInteger, 4)
   state = ps.SparseState()

   ps.Init_Unsafe("reg", 5)(state)
   ps.QFT("reg")(state)        # 应用 QFT
   ps.inverseQFT("reg")(state)  # 应用逆 QFT，恢复到 |5⟩
   ps.print(state)
   # 输出：
   # StatePrint (mode=Detail)
   # |(0)reg : UInt4 |
   # 1.000000+0.000000i  reg=|5>   ← QFT 后再 inverseQFT 恢复到 |5⟩


示例 3：Python 侧自定义算子
----------------------------

当内置算子不满足需求时，可以在 Python 侧直接**组合已有算子**封装为新类：

.. code-block:: python

   class MyDoubleAdder:
       """将目标寄存器的值乘以 2，左移后加到自身。"""

       def __init__(self, target: str, control: str):
           self.target = target
           self.control = control

       def __call__(self, state: ps.SparseState):
           # 复制一份 target
           ps.SplitRegister(self.target, "tmp", 1)(state)
           ps.CombineRegister(self.target, "tmp")(state)
           # tmp = target << 1
           ps.ShiftLeft(self.target, 1)(state)
           # target = target + tmp
           ps.Add_UInt_UInt(self.target, "tmp")(state)
           ps.RemoveRegister("tmp")(state)

       def dag(self, state: ps.SparseState):
           # 对于自伴算子，dag = 自身
           self(state)


对于需要新 primitives 的情况，``pysparq.dynamic_operator.compile_operator`` 支持将用户提供的 C++ 代码编译为动态链接库，并直接包装为 Python 类：

.. code-block:: python

   from pysparq.dynamic_operator import compile_operator

   cpp_code = '''
   class FlipOp : public SelfAdjointOperator {
       size_t reg_id;
   public:
       FlipOp(size_t r) : reg_id(r) {}
       void operator()(std::vector<System>& state) const override {
           for (auto& s : state) {
               s.get(reg_id).value ^= 1;
           }
       }
   };
   '''

   FlipOp = compile_operator(
       name="FlipOp",
       cpp_code=cpp_code,
       base_class="SelfAdjointOperator",
       constructor_args=[("size_t", "reg_id")],
       verbose=True,   # 查看编译输出
   )

   # 使用方式与内置算子完全一致
   ps.System.clear()
   ps.System.add_register("q", ps.UnsignedInteger, 4)
   state = ps.SparseState()
   ps.Init_Unsafe("q", 1)(state)

   flip = FlipOp(reg_id=0)
   flip(state)          # 第 0 比特翻转
   flip.dag(state)      # 逆操作（FlipOp 自伴，dag = 自身）

编译结果基于代码哈希缓存，重复调用不会重新编译。

如需更精细的编译器控制（如添加自定义头文件、链接额外库），参见 :doc:`dynamic_operators`。

示例 4：Block Encoding 概念与实现
-----------------------------------

Block Encoding 是量子算法中最重要的电路构建范式之一——它将一个 :math:`d \times d` 的经典（或量子）矩阵 :math:`A` 编码为一个大得多的酉矩阵 :math:`U_A`，使得：

.. math::

   \langle 0| \langle i| U_A |0\rangle |j\rangle = A_{ij} / \alpha

其中 :math:`\alpha` 是归一化因子。

三对角矩阵的 Block Encoding
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

``pysparq.algorithms.block_encoding.BlockEncodingTridiagonal`` 将对称三对角矩阵 :math:`A = \alpha I + \beta T`（:math:`T` 为移位矩阵）编码为量子电路。实现逻辑：

1. 在辅助寄存器上准备 4 元素叠加态
2. 对主寄存器执行受控加/减 1（溢出比特记录进位）
3. 当 :math:`\beta < 0` 时插入反射门修正符号
4. 逆算子用于释放辅助寄存器

完整示例：

.. code-block:: python

   import numpy as np
   import pysparq as ps
   from pysparq.algorithms.block_encoding import (
       get_tridiagonal_matrix,
       BlockEncodingTridiagonal,
   )

   alpha, beta, dim = 0.5, 0.3, 4
   A = get_tridiagonal_matrix(alpha, beta, dim)
   print(f"三对角矩阵 A:\n{A}")

   # --- 构建 Block Encoding 电路 ---
   ps.System.clear()
   ps.System.add_register("main_reg", ps.UnsignedInteger, 2)  # dim = 2^2 = 4
   ps.System.add_register("anc_UA", ps.UnsignedInteger, 4)

   state = ps.SparseState()
   ps.Init_Unsafe("main_reg", 0)(state)
   ps.Init_Unsafe("anc_UA", 0)(state)

   block_enc = BlockEncodingTridiagonal("main_reg", "anc_UA", alpha, beta)
   block_enc(state)
   print(f"Block Encoding 应用成功，当前基态数: {state.size()}")

   # 逆 Block Encoding（释放辅助寄存器）
   block_enc.dag(state)
   print(f"逆 Block Encoding 后基态数: {state.size()}")


QRAM-Based Block Encoding
~~~~~~~~~~~~~~~~~~~~~~~~~~

对于任意稀疏矩阵（存储在 QRAM 中），``BlockEncodingViaQRAM`` 组合 :math:`U_L`（行方向旋转）、:math:`U_R^\dagger`（列方向旋转）和 SWAP 操作，实现完整的 Block Encoding：

.. math::

   U_A = \text{SWAP}(\text{row}, \text{col}) \cdot U_R^\dagger(\text{col}) \cdot U_L(\text{row}, \text{col})

各操作均通过 QRAM 加载父/子节点数据、计算旋转角、执行条件旋转来实现。具体实现见
``pysparq.algorithms.block_encoding`` 模块源码。

这两个 Block Encoding 构建块是 QDA（量子线性系统求解器）和 Hamiltonian Simulation 等高级算法的核心，构成了从算子到完整量子算法的桥梁。


示例 5：C++ 侧自定义算子（进阶）
----------------------------------

如果 Python 侧的性能或表达能力不足，可以直接在 C++ 核心中实现新算子，步骤如下：

1. **在** ``SparQ/include/`` **创建头文件**，继承 ``BaseOperator`` 或 ``SelfAdjointOperator``，实现 ``apply()`` 方法：

   .. code-block:: cpp

      // SparQ/include/MyCustomOp.h
      #pragma once
      #include "base_operator.h"

      class MyCustomOp : public SelfAdjointOperator {
      public:
          MyCustomOp(size_t reg_id, double phase)
              : reg_id_(reg_id), phase_(phase) {}

          void apply(std::vector<System>& state) const override {
              for (auto& s : state) {
                  // 读取寄存器值
                  auto val = s.get(reg_id_).value;
                  // 修改振幅或寄存器值
                  s.amplitude *= std::exp(complex(0, phase_ * val));
              }
          }

      private:
          size_t reg_id_;
          double phase_;
      };

2. **在** ``SparQ/src/`` **实现** ``apply()`` **方法**（如需单独 cpp 文件）

3. **在** ``PySparQ/src/pybind_wrapper.cpp`` **添加 pybind11 绑定**

4. **重新构建 PySparQ**（``pip install .``）

完整的开发流程参见 :doc:`dynamic_operators`。
