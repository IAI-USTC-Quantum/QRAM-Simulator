QRAM 算子
=========

QRAM (Quantum Random Access Memory) 算子实现量子并行数据访问。

.. contents:: 目录
   :local:
   :class: this-will-duplicate-information-and-it-is-still-useful-here

概述
----

.. list-table:: QRAM 算子总览
   :header-rows: 1

   * - 算子
     - 操作
     - 幺正类
   * - ``QRAMLoad``
     - 标准 QRAM 加载
     - SelfAdjoint
   * - ``QRAMLoadFast``
     - 优化 QRAM 加载
     - SelfAdjoint

量子并行数据访问
----------------

QRAM 实现量子并行存储访问：

.. math::

   \sum_x \alpha_x |x\rangle |0\rangle \xrightarrow{QRAM} \sum_x \alpha_x |x\rangle |f(x)\rangle

这使得量子算法可以同时访问多个数据项。

---

QRAMLoad
--------

.. autoclass:: pysparq.QRAMLoad
   :members:
   :undoc-members:

**操作**: 从 QRAM 电路加载数据到量子寄存器

**类型约束**:
- 地址寄存器: ``UnsignedInteger``
- 数据寄存器: ``UnsignedInteger`` 或 ``General``

**位约束**:
- 地址大小必须匹配 QRAM 配置
- 数据大小必须匹配 QRAM 数据宽度

.. code-block:: python

   import pysparq as ps

   ps.System.clear()

   # 创建 QRAM 电路（qutrit 版本）
   addr_size = 4  # 4 位地址 → 16 个存储单元
   data_size = 8  # 8 位数据

   # 设置存储内容（memory 需在构造时传入）
   memory = [i * 10 for i in range(16)]  # 0, 10, 20, ..., 150
   qram = ps.QRAMCircuit_qutrit(addr_size, data_size, memory)

   # 创建寄存器
   ps.System.add_register("addr", ps.UnsignedInteger, addr_size)
   ps.System.add_register("data", ps.UnsignedInteger, data_size)

   state = ps.SparseState()

   # 地址寄存器均匀叠加
   ps.Hadamard_Int_Full("addr")(state)

   # QRAM 加载：并行访问所有地址
   ps.QRAMLoad(qram, "addr", "data")(state)

   ps.pprint(state)
   # 输出包含 16 个状态：
   # |addr=0,data=0⟩, |addr=1,data=10⟩, ...

   # QRAMLoad 是自伴的，再次应用撤销
   ps.QRAMLoad(qram, "addr", "data")(state)
   # 数据寄存器清零

QRAMLoadFast
------------

.. autoclass:: pysparq.QRAMLoadFast
   :members:
   :undoc-members:

**操作**: 优化的 QRAM 加载

**特点**: 针特定存储模式优化，性能更高。

.. code-block:: python

   # 快速加载（相同接口）
   ps.QRAMLoadFast(qram, "addr", "data")(state)

---

QRAMCircuit_qutrit
------------------

PySparQ 提供 ``QRAMCircuit_qutrit`` 类用于配置 QRAM：

.. autoclass:: pysparq.QRAMCircuit_qutrit
   :members:
   :undoc-members:

.. note::

   ``QRAMCircuit_qubit``（C++ API 中的 qubit 版本 QRAM）在 PySparQ 中**不可用**。PySparQ 仅提供 qutrit 版本。

创建和配置 QRAM 电路
^^^^^^^^^^^^^^^^^^^^^

.. code-block:: python

   import pysparq as ps

   # 创建 QRAM 电路
   addr_bits = 4   # 地址位数
   data_bits = 8   # 数据位数

   # 设置存储内容（memory 需在构造时传入）
   memory_list = [0, 10, 20, 30, 40, 50, 60, 70,
                  80, 90, 100, 110, 120, 130, 140, 150]
   qram = ps.QRAMCircuit_qutrit(addr_bits, data_bits, memory_list)

   # 注意：set_memory() 方法在 PySparQ 中不可用
   # memory 必须在构造 QRAMCircuit_qutrit 时传入

   # 查询信息
   print(f"地址大小: {qram.addr_size}")
   print(f"数据大小: {qram.data_size}")

条件加载
--------

QRAM 加载支持条件执行：

.. code-block:: python

   op = ps.QRAMLoad(qram, "addr", "data")

   # 仅当 control 非零时加载
   op.conditioned_by_nonzeros("control")(state)

   # 仅当 flag 的第 0 位为 1 时加载
   op.conditioned_by_bit("flag", 0)(state)

使用场景
--------

量子数据库搜索
^^^^^^^^^^^^^^

.. code-block:: python

   # 数据库内容
   database = [42, 17, 99, 5, ...]

   # 创建 QRAM（memory 在构造时传入）
   qram = ps.QRAMCircuit_qutrit(addr_bits, data_bits, database)

   # 地址叠加态
   ps.Hadamard_Int_Full("addr")(state)

   # 并行加载所有数据项
   ps.QRAMLoad(qram, "addr", "data")(state)

   # 现在可以搜索特定值...

量子机器学习
^^^^^^^^^^^^

.. code-block:: python

   # 加载训练数据（memory 在构造时传入）
   qram = ps.QRAMCircuit_qutrit(addr_bits, data_bits, training_data)
   ps.Hadamard_Int_Full("sample_id")(state)
   ps.QRAMLoad(qram, "sample_id", "sample_data")(state)

   # 现在可以并行处理所有训练样本