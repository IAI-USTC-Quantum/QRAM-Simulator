Shor 量子因式分解算法
=====================

.. contents:: Table of Contents
   :local:
   :class: this-will-duplicate-information-and-it-is-still-useful-here

概述
----

Shor 算法为整数分解提供指数级加速，理论上可破解 RSA 加密。给定合数 N，算法使用量子周期查找在多项式时间内找到因子。

本教程演示使用 PySparQ 的两种实现：

1. **半经典 Shor**：实用的迭代测量方法
2. **全量子 Shor**：教科书式量子相位估计

数学背景
--------

算法原理
~~~~~~~~

Shor 算法通过以下步骤分解 N：

1. **经典预处理**：选取与 N 互质的随机数 a
2. **量子周期查找**：找到 r 使得 a^r ≡ 1 (mod N)
3. **经典后处理**：计算 gcd(a^(r/2) ± 1, N)

周期查找
~~~~~~~~

量子部分查找函数的周期 r：

.. math::

   f(x) = a^x \mod N

在酉算子上使用量子相位估计：

.. math::

   U|x\rangle = |x \oplus 1\rangle

U 的本征值为 e^(2πik/r)，给出周期。

连分数
~~~~~~

测量结果 y/Q 通过以下关系与 c/r 相关：

.. math::

   \frac{y}{Q} \approx \frac{c}{r}

其中 Q = 2^size（精度寄存器大小）。

我们使用连分数展开（法里序列）提取 r。

实现步骤
--------

步骤 1：导入和设置
~~~~~~~~~~~~~~~~~~

.. code-block:: python

   import pysparq as ps
   from pysparq.algorithms.shor import factor, SemiClassicalShor, general_expmod

   import math
   import random

步骤 2：经典辅助函数
~~~~~~~~~~~~~~~~~~~

模幂函数：

.. code-block:: python

   def general_expmod(a: int, x: int, N: int) -> int:
       """使用平方-乘法计算 a^x mod N。"""
       if x == 0:
           return 1
       if x == 1:
           return a % N
       if x & 1:  # 奇数
           return (general_expmod(a, x - 1, N) * a) % N
       else:  # 偶数
           half = general_expmod(a, x // 2, N)
           return (half * half) % N

   # 示例用法
   result = general_expmod(7, 5, 15)  # 7^5 mod 15 = 7
   print(f"7^5 mod 15 = {result}")

步骤 3：连分数展开
~~~~~~~~~~~~~~~~~

从测量中提取周期：

.. code-block:: python

   from fractions import Fraction

   def find_best_fraction(y: int, Q: int, N: int):
       """使用法里序列找到最佳分数 c/r ≈ y/Q。"""
       target = y / Q

       # 通过法里序列进行二分搜索
       low_num, low_den = 0, 1
       high_num, high_den = 1, 1
       best_num, best_den = 0, 1

       while low_den + high_den <= N:
           mediant_num = low_num + high_num
           mediant_den = low_den + high_den

           if mediant_num / mediant_den < target:
               low_num, low_den = mediant_num, mediant_den
           else:
               high_num, high_den = mediant_num, mediant_den

           # 跟踪最佳逼近
           if abs(mediant_num/mediant_den - target) < abs(best_num/best_den - target):
               best_num, best_den = mediant_num, mediant_den

       return best_den, best_num  # (r, c)

步骤 4：半经典 Shor 电路
~~~~~~~~~~~~~~~~~~~~~~~~

构建迭代测量电路：

.. code-block:: python

   class SemiClassicalShor:
       def __init__(self, a: int, N: int):
           if math.gcd(a, N) != 1:
               raise ValueError("a 和 N 必须互质")

           self.a = a
           self.N = N
           self.n = int(math.log2(N)) + 1
           self.size = self.n * 2  # 精度寄存器大小

       def run(self):
           ps.System.clear()
           state = ps.SparseState()

           # 创建初始化为 |1> 的辅助寄存器
           anc_reg = ps.AddRegister("anc_reg", ps.UnsignedInteger, self.n)(state)
           ps.Xgate_Bool("anc_reg", 0)(state)

           results = []

           # 迭代相位估计
           for x in range(self.size):
               # 工作量子比特在叠加态
               work_reg = ps.AddRegisterWithHadamard(
                   "work_reg", ps.UnsignedInteger, 1
               )(state)

               # 受控模乘
               modmul = ps.Mod_Mult_UInt_ConstUInt("anc_reg", self.a, self.size - 1 - x, self.N)
               modmul.conditioned_by_all_ones("work_reg")(state)

               # 来自之前测量的相位修正
               for i, result in enumerate(reversed(results)):
                   if result == 1:
                       phase = -2 * math.pi / (2 ** (i + 2))
                       ps.Phase_Bool("work_reg", phase)(state)

               # 测量工作量子比特
               ps.Hadamard_Bool("work_reg")(state)
               measured, _ = ps.PartialTrace(["work_reg"])(state)
               results.append(measured[0])

               ps.RemoveRegister("work_reg")(state)

           # 将比特结果转换为整数
           meas_result = sum(bit * (2**i) for i, bit in enumerate(results))

           return meas_result, results

步骤 5：经典后处理
~~~~~~~~~~~~~~~~~

从测量结果提取因子：

.. code-block:: python

   def shor_postprocess(meas: int, size: int, a: int, N: int):
       """从测量结果提取因子。"""
       Q = 2 ** size
       r, c = find_best_fraction(meas, Q, N)

       # 验证周期
       if r > N:
           raise ValueError("周期过大")
       if r % 2 == 1:
           raise ValueError("奇数周期")

       # 检查 a^(r/2) ≠ -1 mod N
       a_r_half = general_expmod(a, r // 2, N)
       if a_r_half == N - 1:
           raise ValueError("a^(r/2) = -1 mod N")

       # 计算因子
       p = math.gcd(a_r_half + 1, N)
       q = math.gcd(a_r_half - 1, N)

       return p, q

步骤 6：完整分解
~~~~~~~~~~~~~~~

.. code-block:: python

   def factor(N: int, a: int = None):
       """使用 Shor 算法分解 N。"""
       if N % 2 == 0:
           return (2, N // 2)

       if a is None:
           a = random.randint(2, N - 1)

       # 检查是否已经是因子
       g = math.gcd(a, N)
       if g != 1:
           return (g, N // g)

       shor = SemiClassicalShor(a, N)
       p, q = shor.run()

       return p, q

完整示例
--------

.. code-block:: python

   from pysparq.algorithms.shor import factor

   # 分解 15
   p, q = factor(15)
   print(f"15 的因子: {p} 和 {q}")
   print(f"验证: {p} × {q} = {p * q}")

   # 输出:
   # 分解 N = 15，a = 2
   # 找到周期: 4, c: 1
   # a^(r/2) mod N = 4
   # p = 5, q = 3, p * q = 15
   # 15 的因子: 5 和 3
   # 验证: 5 × 3 = 15

多次尝试
~~~~~~~~

.. code-block:: python

   import random

   N = 21

   # 可能需要多次尝试
   for attempt in range(5):
       a = random.randint(2, N - 1)
       if math.gcd(a, N) == 1:
           try:
               p, q = factor(N, a)
               if p != 1 and q != 1 and p * q == N:
                   print(f"成功! {N} = {p} × {q}")
                   break
           except ValueError as e:
               print(f"尝试 {attempt+1} 失败: {e}")

理解量子电路
------------

寄存器布局
~~~~~~~~~~

分解 N 时：

- **辅助寄存器**（n = ceil(log2(N)) 比特）：保存 y → y * a^(2^x) mod N
- **工作寄存器**（1 比特）：单次迭代测量量子比特

电路使用 2n 次受控模乘迭代。

电路图
~~~~~~

每次迭代 x 在 [0, 2n) 中：

::

   work: |0>--H--[ctrl]--H--measure--> b_x
                     |
   anc:  |1>--[ModMul(a, 2^x, N)]-->

ModMul 操作计算：

.. math::

   |y\rangle \rightarrow |y \cdot a^{2^x} \mod N\rangle

受工作量子比特控制。

相位修正
~~~~~~~~

测量比特 x 后，对比特 x+1, x+2, ... 应用相位修正：

.. math::

   R_z\left(-\frac{2\pi b_j}{2^{j-i}}\right)

这正确实现了迭代相位估计。

API 参考
---------

.. autoclass:: pysparq.algorithms.shor.SemiClassicalShor
   :members:

.. autoclass:: pysparq.algorithms.shor.ModMul
   :members:

.. autoclass:: pysparq.algorithms.shor.Shor
   :members:

.. autofunction:: pysparq.algorithms.shor.factor

.. autofunction:: pysparq.algorithms.shor.general_expmod

.. autofunction:: pysparq.algorithms.shor.find_best_fraction
