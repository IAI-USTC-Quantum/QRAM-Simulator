PySparQ 文档
============

PySparQ 是一个稀疏态量子电路模拟器，具有原生 QRAM 支持和寄存器级编程范式。

.. raw:: html

   <div class="badges" style="display: flex; gap: 10px; flex-wrap: wrap; margin-bottom: 20px;">
     <a href="https://arxiv.org/abs/2503.13832"><img src="https://img.shields.io/badge/QRAM_Simulator-arXiv%3A2503%2E13832-b31b1b.svg" alt="arXiv"></a>
     <a href="https://arxiv.org/abs/2503.15118"><img src="https://img.shields.io/badge/SparQ-arXiv%3A2503%2E15118-6f42c1.svg" alt="arXiv"></a>
     <a href="https://pypi.org/project/pysparq/"><img src="https://img.shields.io/pypi/v/pysparq.svg" alt="PyPI"></a>
     <a href="https://github.com/IAI-USTC-Quantum/QRAM-Simulator"><img src="https://img.shields.io/badge/GitHub-Repo-181717?logo=github" alt="GitHub"></a>
     <a href="https://iai-ustc-quantum.github.io/QRAM-Simulator/"><img src="https://img.shields.io/badge/docs-GitHub%20Pages-4D6AE4" alt="Documentation"></a>
   </div>

快速链接
--------

* `GitHub 仓库 <https://github.com/IAI-USTC-Quantum/QRAM-Simulator>`_ - 源码和问题反馈
* `主页 <https://iai-ustc-quantum.github.io/QRAM-Simulator/>`_ - 项目概览和 C++ API 文档

.. toctree::
   :maxdepth: 3
   :caption: 用户指南

   guide/installation
   guide/quickstart
   guide/architecture
   guide/examples
   guide/dynamic_operators
   guide/core_concepts/index
   guide/development/index

.. toctree::
   :maxdepth: 2
   :caption: 算子参考

   operators/index

.. toctree::
   :maxdepth: 2
   :caption: 交互教程

   notebooks/01_快速入门
   notebooks/02_稀疏态演化
   notebooks/03_算子示例

.. toctree::
   :maxdepth: 2
   :caption: API 参考

   api/index

快速开始
--------

通过 pip 安装 PySparQ：

.. code-block:: bash

   pip install pysparq

快速示例：

.. code-block:: python

   from pysparq import System, SparseState, AddRegister, Hadamard_Int

   system = System()
   state = SparseState(system)
   AddRegister("q", pysparq.UnsignedInteger, 4)(state)
   Hadamard_Int("q")(state)
   print(state)

索引和表格
==========

* :ref:`genindex`
* :ref:`modindex`
* :ref:`search`
