API 参考
========

.. toctree::
   :maxdepth: 2

   cpp

Python 绑定（``qram_simulator`` 包）的 API 由 sphinx-autoapi 从
pybind11-stubgen 生成的类型桩自动构建；在线站点上见左侧
"api/python" 章节。docstring 与本页 C++ 注释同源（Doxygen），
如需查阅完整注释亦可直接阅读仓库头文件：

- ``QRAM/include/`` —— 两套 QRAM 电路与分支结构
- ``Common/include/`` —— 全振幅桥接、随机引擎、矩阵、日志等基础设施
