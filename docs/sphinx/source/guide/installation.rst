安装
====

环境要求
--------

- Python 3.10 或更高版本
- NumPy

从 PyPI 安装
------------

.. code-block:: bash

   pip install pysparq

从源码安装
----------

.. code-block:: bash

   git clone https://github.com/IAI-USTC-Quantum/QRAM-Simulator.git
   cd QRAM-Simulator
   pip install .

编译要求
--------

从源码编译需要以下工具：

- CMake 3.15+
- 支持 C++17 的编译器
- OpenMP（用于并行化，必需）

可选依赖：

- CUDA/GPU 后端当前在 CMake 中临时屏蔽；源码构建默认走 CPU-only 路径。

验证安装
--------

.. code-block:: python

   import pysparq
   pysparq.test_import()

   # 创建量子系统
   system = pysparq.System()
   state = pysparq.SparseState(system)

   print("PySparQ 安装成功！")
