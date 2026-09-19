# 面向 Qubit 架构 QRAM 的分支剪枝与快速模拟：H–K₀–H 可预测性理论

> 理论设计文档（预实现）。对应主文献：Yun-Jie Wang 等，*Efficient Simulation of Quantum Random Access Memory*（arXiv:2503.13832，Phys. Rev. Applied **25**, 044069）。本文解决该工作遗留的一个缺口：**qubit 编码（standard）bucket-brigade QRAM 在振幅阻尼噪声下的分支预测与剪枝**。

## 摘要

稀疏态 QRAM 模拟器（本仓库）对 qutrit 编码的 bucket-brigade QRAM 实现了含噪快速模拟：噪声历史被预先采样为一棵"分支树"，路由路径不经过任何噪声点的 good 分支不做真实演化，其末态振幅由解析公式直接预测；只有 good/bad 划分由子树包含判据预先确定的 bad 分支需要显式演化。该技巧在 qutrit 方案中之所以简单，是因为数据总线在计算基中传输，无跳变 Kraus 算子 $K_0$ 在其上是对角的，good 分支的全部阻尼效应退化为一个可数的标量衰减因子。然而在 qubit 编码方案中，数据比特必须以 $\{|+\rangle,|-\rangle\}$ 基穿过整棵树（CZ 相位取数协议要求先作用 Hadamard），$K_0=\mathrm{diag}(1,a)$ 在 X 基下非对角，标量预测失效。本文证明：这一 $H\to K_0^d\to H$ 结构依然是**闭式可预测**的——第二个 H 门所作用的正是 $(|0\rangle+(-1)^{b\oplus m}a^d|1\rangle)/\sqrt{2}$ 形式的态，输出振幅为 $(1\pm a^d)/2$；$k$ 个数据比特给出 $2^k$ 个只依赖错误图样 Hamming 权重的闭式分量。据此我们给出完整的 qubit-QRAM 剪枝算法，保持与 qutrit 方案相同的复杂度标度 $\mathcal{O}(d+p\cdot\mathrm{poly}(n))$，并阐明其误差结构：good 分支的一阶效应仅为范数流失（被蒙特卡洛轨迹权重自然吸收），真正的分支内误差是二阶的相干泄漏，一阶错误全部集中于被显式演化的 bad 分支。

---

## 1. 引言

量子随机存取存储器（QRAM）以 $\mathcal{O}(\log N)$ 的查询深度实现叠加寻址的数据读取，是量子数据库搜索、量子态制备、量子机器学习等算法的核心部件；其代价是需要 $\mathcal{O}(N)$ 个物理量子比特的二叉路由树。Wang 等人 [1] 提出的稀疏态模拟器利用两个观察实现了 QRAM 的高效经典模拟：(i) bucket-brigade 电路全部由非分支（non-branching）可逆门构成，每个输入基态分量的树配置演化是确定的、唯一的；(ii) 含噪演化可按蒙特卡洛轨迹展开，每个噪声点只影响路由路径经过它的那些分支（子树包含），其余 good 分支的末态可以解析预测而不必演化。模拟复杂度从全密度矩阵的指数级降到 $\mathcal{O}(d+p\cdot\mathrm{poly}(n))$（$d$ 为输入叠加分支数，$p$ 为单比特噪声率，$n$ 为地址位数）。

该工作及其代码实现以 **qutrit 编码**为主力方案：路由节点取三能级 $\{|W\rangle,|L\rangle,|R\rangle\}$，空闲态 $|W\rangle$ 是阻尼信道无跳变算子 $K_0$ 的不动点，数据传输在计算基中完成。对于物理上更常见的 **qubit 编码**（每个路由节点两个二能级比特，地址比特 $|0\rangle\equiv L$、$|1\rangle\equiv R$，无空闲态），快速模拟的支持一直不完整：仓库中 qubit 版好分支乘子只实现了地址路由部分的标量计数（`TimeStep::_get_multiplier_impl_qubit`，且接线代码处于注释停用状态，见 `QRAM/include/qram_branch_qubit.h:213-225`），而数据总线部分——正是本文主题——缺少理论公式。

qubit 方案与 qutrit 方案的本质区别在于数据取数方式 [2]：qutrit 方案用经典控制的 CNOT 在计算基翻转数据比特；qubit 方案用 **CZ 相位编码**——经典存储位作为控制，对途经叶子的数据比特施加 $Z$。为了让"激活节点的 $|0\rangle$"与"未激活节点"可区分，数据总线进入树之前必须先作用 Hadamard，把 $\{|0\rangle,|1\rangle\}$ 换到 $\{|+\rangle,|-\rangle\}$ 基，取出数据返回后再作用一次 H 换回原基。于是每个数据比特在两次 H 之间以 X 基叠加态穿行整棵树，持续承受振幅阻尼的 $K_0$ 作用——这就是本文分析的 **$H\to K_0^d\to H$ 结构**。

本文的贡献：

1. 证明 qubit-QRAM good 分支的 $H\to K_0^d\to H$ 结构具有闭式解（引理 1），并给出 $k$ 数据比特下的 $2^k$ 分量 Hamming 权重公式（定理 2）；
2. 给出 good 分支末态的完整预测定理（定理 3），将 Wang 等人 v2 版论文中"所有可靠分支共享同一末态"的命题 [1, Eq. (6)] 推广到 qubit 编码；
3. 分析误差结构：good 分支输出相对理想输出的偏差分为一阶范数流失与二阶相干泄漏两部分，前者不是误差而是跳变轨迹的概率权重；
4. 给出完整的剪枝快速模拟算法与复杂度分析，并对照本仓库列出现有实现缺口。

## 2. 背景与记号

### 2.1 $(n,k)$-QRAM 与时间片调度

$(n,k)$-QRAM 实现变换
$$
\sum_{i,j}\alpha_{i,j}|i\rangle_A|j\rangle_D\;\longrightarrow\;\sum_{i,j}\alpha_{i,j}|i\rangle_A|j\oplus d_i\rangle_D,
$$
其中 $n$ 为地址比特数（$N=2^n$），$k$ 为数据比特数，$d_i\in\{0,1\}^k$ 为经典存储内容 [1,2]。

bucket-brigade 查询在一棵深度 $n$ 的全二叉树上流水执行：地址比特逐位注入根节点并下钻建立路由路径，数据总线比特逐个换入（CopyIn）、沿路径下行、在叶子与存储作用后原路返回、换出（CopyOut），最后全部操作逆序执行以解除纠缠（uncompute）。模拟器把电路组织为 $T=6n+2k$ 个时间片（`TimeStep::full_step`，`QRAM/src/time_step.cpp:118-121`），关键调度点为（`time_step.cpp:133-203`）：

| 事件 | 数据比特 $i$ 的时间片 |
|---|---|
| 地址比特 $t$ 注入（ACopy） | $2t+1$ |
| CopyIn$_i$（数据比特 $i$ 换入根节点） | $\tau_{\mathrm{in}}(i)=2n+2i+1$ |
| FetchData$_i$（叶子处 CZ 取数） | $\tau_{\mathrm{fetch}}(i)=3n+2i+1$ |
| CopyOut$_i$（数据比特 $i$ 换出） | $\tau_{\mathrm{out}}(i)=4n+2i+1$ |

调度关于中线镜像对称：$\mathrm{out}(t)=T-t$。注意 $\tau_{\mathrm{out}}(i)-\tau_{\mathrm{in}}(i)=2n$ 与 $i$ 无关，且 $\tau_{\mathrm{fetch}}(i)$ 恰为窗口中点。在 qubit 架构中，两片 Hadamard 分别挂接在 CopyIn$_0$ 之前与 CopyOut$_{k-1}$ 之后（`QRAM/include/qram_circuit_qubit.h:1039-1048`），作用于整个数据总线寄存器。

### 2.2 噪声模型与蒙特卡洛轨迹展开

每个时间片的门操作之后，对当前已纠缠子树内的每个物理比特独立地以概率施加噪声（`TimeStep::noise_one_step`，`time_step.cpp:341-385`）：

- **振幅阻尼**（强度 $\gamma$）：Kraus 算子
$$
K_0=|0\rangle\langle0|+\sqrt{1-\gamma}\,|1\rangle\langle1|=\mathrm{diag}(1,a),\qquad K_1=\sqrt{\gamma}\,|0\rangle\langle1|,
$$
记 $a\equiv\sqrt{1-\gamma}$ 为单步衰减因子。模拟中拆为两部分：$K_0$ 作为**默认算子**确定性作用（`Damp_Common`：每步将每个处于 $|1\rangle$ 的激发振幅乘以 $a$，`qram_branch_qubit.h:84-87`）；跳变算子 $K_1$ 只在预先采样的噪声点处作为**准测量**处理（`Damp_Full`：按当前未归一化态计算跳变概率、掷点、命中则对所有分支投影复位，`qram_circuit_qubit.h:468-558`）。
- **退极化**（强度 $\varepsilon$）：随机 Pauli $X/Z/Y$，在采样到的噪声点处显式施加。

一条轨迹 = 一份预先采样的噪声历史（噪声点位置、种类）+ 默认 $K_0$ 的全程作用。状态始终保持为（未归一化的）稀疏纯态，只在准测量与末态采样处归一化 [1, SI §III]。多轨迹平均即还原密度矩阵层面的观测量。

### 2.3 qutrit 方案的三条可预测性支柱

Wang 等人的剪枝算法建立在三条支柱上 [1]：

**(P1) 唯一树配置定理。** 电路线全由 CSWAP/SWAP/CNOT/CZ 等非分支门构成（只置换计算基态、不产生叠加），因此对每个地址 $|i\rangle$ 存在唯一的树配置轨道 $|\Psi_i(t)\rangle$；无噪演化无需解方程即可经典推演。

**(P2) 默认算子的对角性。** $K_0$ 在节点计算基下对角，故无跳变轨迹中 good 分支的基态轨道与无噪轨道完全相同，阻尼的全部效应是乘上一个**可数的标量衰减因子**：振幅因子 $a^{c}$，其中 $c$ 为该分支的"激发数 × 时间步"计数，由调度表解析给出而无需模拟。不同 good 分支之间的相对权重为 $a^{\Delta c}$（实现：`TimeStep::get_multiplier_qutrit`，`QRAM/include/time_step.h:209-234`）。

**(P3) 子树包含判据。** 节点 $(l,p)$ 处的故障只影响其叶子子树对应的地址区间
$$
i\in\big[2^{\,n-l}p,\;2^{\,n-l}(p+1)-1\big],
$$
故给定噪声历史后，good/bad 分支集合可**在演化之前**解析确定（实现：`get_bad_range_qutrit` + 区间并集 `ContinuousRange`，`time_step.cpp:267-339`）。期望 bad 分支比例为 $\mathcal{O}(n^2p)$ [1, v1 Eq. (6)]。

**注意 (P3) 的编码依赖性（详见姊妹篇 `qubit_error_propagation.md`）。** 该判据对 qutrit 的真正依据不是"输出损坏局限于子树"（这一点对 qubit 也成立），而是"**路径外分量的末态树构型保持同一**"：qutrit 的故障残留被 $|W\rangle$ 守卫（`QRAMState::cswap` 的 `case W: do nothing`）冻结在故障节点原地，所有路径外分量共享同一构型，可作 good 分支预测。qubit 编码没有这个守卫，X 型故障的残留会沿"空闲=指向左"的祖先链**迁移上拉**，使构型家族在更大范围内分歧——坏区间必须改用左孩子上溯规则（§3、§5.1）。

qutrit 方案中数据传输在计算基完成、不需要 H 门，两片 H 的挂接点退化为纯合并操作（`qram_branch_qutrit.cpp:547-573` 的 `run_hadamard` 只做 `try_merge`），因此 (P2) 直接覆盖数据总线：good 分支不分裂，末态为 $|i\rangle|j\oplus d_i\rangle|Q_0\rangle$ 乘标量因子。

## 3. qubit 方案的缺口

qubit 编码中支柱 (P1) 逐字成立（非分支门集与能级编码无关）。缺口有两处——(P2) 的数据总线部分，以及 (P3) 的构型同一性：

- **(P3′) 构型家族分歧（X 型故障）**：输出损坏（wrongbus）本身仍 ⊆ subtree(v)（单错误注入实测），但 X 翻转在**所有**叠加分量留下永久残留（空闲节点无人清理），且残留会被"空闲=指向左"的祖先上拉迁移：左孩子位置的残留对父节点空闲的分量上溯一级，右孩子位置的残留被困于自身子树。路径外分量因此分裂为不同末态构型家族，家族分歧的包络为 **bad = 左孩子取 subtree(parent(v))，右孩子取 subtree(v)**（即现行 `get_bad_range_qubit`；根与根的孩子有特判）。阻尼跳变（K₁）不产生路径外残留（空闲无激发可跳），**纯阻尼通道仍可用纯子树判据**。机制推导与 n=3 全注入验证见 `qubit_error_propagation.md`。
- **(P2) 地址/路由部分**：路由比特处于 $|1\rangle$ 时同样被 $K_0$ 逐步衰减，暴露窗口由调度表给出，仍是可数标量（仓库已实现 `_get_multiplier_impl_qubit`，`time_step.cpp:462-475`）。这部分没有新困难。
- **(P2) 数据总线部分**：数据比特 $i$ 在第 $\tau_{\mathrm{in}}(i)=2n+2i+1$ 步换入、第 $\tau_{\mathrm{out}}(i)=4n+2i+1$ 步换出，其间以 $(|0\rangle\pm|1\rangle)/\sqrt{2}$ 的形式存在于树内。$K_0$ 在 X 基下非对角：
$$
K_0|+\rangle=\tfrac{1}{\sqrt2}\big(|0\rangle+a|1\rangle\big),\qquad K_0|-\rangle=\tfrac{1}{\sqrt2}\big(|0\rangle-a|1\rangle\big).
$$
分支内不同数据分量的衰减不同步，两片 H 不再相互抵消——**第二个 H 门的输出不再是确定的计算基态**。这正是 qutrit 版标量乘子公式失效、也是 naive 实现必须让分支真实演化（每次 H 分裂、逐步计数、末态合并）的原因。

## 4. 主要理论结果

### 4.1 H–K₀–H 引理

**引理 1（单比特闭式）。** 设 $a=\sqrt{1-\gamma}$，$d\in\mathbb{N}$，$m\in\{0,1\}$，定义
$$
N_{m,d}\;=\;H\,\underbrace{Z^{m}\,\mathrm{diag}(1,a^{d})}_{\text{对角, 与 }K_0\text{ 幂次可合并}}\,H .
$$
则对任意输入比特 $b\in\{0,1\}$，记 $b_{\mathrm{out}}=b\oplus m$：
$$
N_{m,d}\,|b\rangle\;=\;\alpha_d\,|b_{\mathrm{out}}\rangle+\beta_d\,|\overline{b_{\mathrm{out}}}\rangle,\qquad
\alpha_d=\frac{1+a^{d}}{2},\;\;\beta_d=\frac{1-a^{d}}{2}.
$$

**证明。** $H|b\rangle=\big(|0\rangle+(-1)^{b}|1\rangle\big)/\sqrt2$。两片 H 之间，对该比特的作用只有：(i) $d$ 次 $K_0=\mathrm{diag}(1,a)$（路由 SWAP 只重定位比特、不改变其状态，见下）；(ii) 叶子处 CZ 取数贡献的 $Z^m$。二者皆在计算基对角、互相对易，故合并为 $Z^m\mathrm{diag}(1,a^d)=\mathrm{diag}\big(1,(-1)^m a^d\big)$。作用后状态为 $\big(|0\rangle+(-1)^{b\oplus m}a^d|1\rangle\big)/\sqrt2$——**这正是"第二个 H 门作用上去的时候，相当于 $|0\rangle+a^d|1\rangle$ 再过一次 H"的精确含义**（符号由输入比特与存储比特共同决定）。再过 $H$：
$$
\frac{1}{2}\Big[\big(1+(-1)^{b\oplus m}a^{d}\big)|0\rangle+\big(1-(-1)^{b\oplus m}a^{d}\big)|1\rangle\Big],
$$
按 $b_{\mathrm{out}}=b\oplus m$ 归并即得结论。$\square$

**注 1（范数守恒的自洽性）。** $\alpha_d^2+\beta_d^2=\frac{1+a^{2d}}{2}$ 恰好等于 X 基态经受 $d$ 层阻尼信道的**无跳变概率** $\frac12(\langle0|+\langle1|)K_0^{d\dagger}K_0^{d}(|0\rangle+|1\rangle)/1$ 的直算结果。缺失的范数 $\frac{1-a^{2d}}{2}$ 正是跳变（$K_1$）轨迹携带的概率权重——在本算法中由 bad 分支显式演化。未归一化框架下两部分的账严格对齐。

**注 2（正确分量与错误分量）。** $\alpha_d$ 是"取数正确"的振幅，$\beta_d$ 是相干泄漏到错误值的振幅。$a=1$（无噪）时 $\beta_d=0$，退化为理想 QRAM 输出 $H^2=I$。

### 4.2 多比特乘积结构

**定理 2（$k$ 数据比特的 $H$–$K_0$–$H$ 公式）。** 在 good 分支 $(i,j)$（地址 $i$、总线输入 $j\in\{0,1\}^k$）上，设数据比特 $i$ 在树内停留 $d_i=\tau_{\mathrm{out}}(i)-\tau_{\mathrm{in}}(i)=2n$ 个阻尼层，存储位 $m_i=(d_i)_{\text{第 }i\text{ 位}}$（记号冲突处以上下文为准，下文记存储内容为 $d$，其第 $t$ 位为 $d^{(t)}$）。则数据寄存器在第二片 H 之后的未归一化状态为
$$
\bigotimes_{t=0}^{k-1}N_{d^{(t)},\,2n}\,|j_t\rangle
\;=\;\sum_{c\in\{0,1\}^k}A(c)\,|c\rangle,
$$
其中输出分量 $c$ 的振幅只依赖于错误图样 $e=c\oplus b_{\mathrm{out}}$（$b_{\mathrm{out}}=j\oplus d$ 为理想输出）的 **Hamming 权重** $w=|e|$：
$$
\boxed{\;A(c)\;=\;\frac{1}{2^{k}}\,(1+a^{2n})^{\,k-w}\,(1-a^{2n})^{\,w}\;=\;\alpha_{2n}^{\,k-w}\,\beta_{2n}^{\,w}\;}
$$

**证明。** 不同数据比特的路由由同一地址路径引导、在不同时间片流水通过，彼此之间无任何门耦合；每比特的取数 $Z^{d^{(t)}}$ 与 $K_0$ 暴露各自独立。故两片 H 之间的总对角算子是逐比特张量积，总映射为 $\bigotimes_t N_{d^{(t)},d_t}$。由调度表，$d_t=\tau_{\mathrm{out}}(t)-\tau_{\mathrm{in}}(t)=(4n+2t+1)-(2n+2t+1)=2n$ 对所有比特相同（端点计数约定至多为 $\pm1$ 的修正，且对全部比特一致，可被参考分支自校准，见 §5.2）。对每个比特应用引理 1，$c_t=b_{\mathrm{out},t}$ 贡献 $\alpha_{2n}$、$c_t=\overline{b_{\mathrm{out},t}}$ 贡献 $\beta_{2n}$，按错误位数 $w$ 归并即得。$\square$

**与 qutrit 计数器的对照值得强调**：qutrit 版数据窗口依赖输入/输出比特值（$b_{\mathrm{in}},b_{\mathrm{out}}$ 决定激发在去程/回程是否存在于树内，见 `_get_multiplier_impl_qutrit` 的数据部分，`time_step.cpp:510-534`），而 qubit 版数据比特**始终以半激发态在树内驻留完整窗口 $2n$**，与比特值无关——X 基传输使暴露窗口变得平凡，这反而简化了计数。

### 4.3 地址路由部分的标量因子

路由比特的激发暴露与 qutrit 情形同属支柱 (P2)。地址 $i$ 的第 $t$ 位为 1 时，对应激发在第 $2t+1$ 步注入、第 $\mathrm{out}(2t+1)=T-(2t+1)$ 步随 uncompute 撤出（qubit 方案无空闲态，路径指针在整个查询期间存活），全轨迹暴露计数为
$$
c_{\mathrm{addr}}(i)=\sum_{t:\,i_t=1}\big(T-2(2t+1)\big)=\sum_{t:\,i_t=1}(6n+2k-4t-2),
$$
与 `_get_multiplier_impl_qubit`（`time_step.cpp:462-475`）一致。整分支振幅获得标量因子 $a^{c_{\mathrm{addr}}(i)}$；不同 good 分支之间的相对概率权重为
$$
\frac{p_i}{p_{i_{\mathrm{ref}}}}=(1-\gamma)^{\,c_{\mathrm{addr}}(i)-c_{\mathrm{addr}}(i_{\mathrm{ref}})}\;\equiv\;a^{2\Delta c},
$$
即 `get_multiplier_qubit` 的 `relative_multiplier`（`time_step.h:186-207`）。

注意 qubit 方案的地址暴露窗口是**全查询程**（$\sim T$ 量级），而 qutrit 方案的地址激发采用传播式激活、每比特仅暴露 $\mathcal{O}(t)$ 步（`time_step.cpp:490-509` 的 $[2t+1,3t+2]$ 及镜像段）——这正是协议层面 qubit 方案 infidelity 标度 $\mathcal{O}((n+k)n^2\varepsilon)$ 比 qutrit 方案 $\mathcal{O}((n+k)n\varepsilon)$ 差一个因子 $n$ 的来源 [2, §III]，与本模拟算法无关但值得在物理解读中注明。

### 4.4 Good 分支末态预测定理

**定理 3（good 分支闭式预测）。** 设噪声历史给定，分支 $(i,j)$ 为 good 分支（其路由路径不经过任何噪声点）。则该分支的无跳变轨迹末态为
$$
|\Psi_{i,j}^{\mathrm{good}}\rangle
\;=\;a^{\,c_{\mathrm{addr}}(i)}\;|i\rangle_A\otimes\Big[\bigotimes_{t=0}^{k-1}N_{d^{(t)},2n}|j_t\rangle\Big]_D\otimes|\tilde Q_0\rangle_{\mathrm{tree}},
$$
其中 $|\tilde Q_0\rangle_{\mathrm{tree}}$ 为（未归一化的）树末态，**与所有 good 分支及参考分支相同**。

**证明。** 由 (P1)，树内可逆门的作用是对唯一基态轨道的重定位；由 (P3)，路径上无噪声点，故树内仅有的非平凡作用是每步的 $K_0$。把 $K_0^{\otimes}$ 按比特分解：路由比特的贡献按 §4.3 计数为标量 $a^{c_{\mathrm{addr}}(i)}$；数据比特的贡献按 §4.1–4.2 归入 $N$ 的闭式（驻留树内的每步贡献一个 $a$，与驻留位置无关，因为 $K_0$ 对各节点比特均匀作用）。树末态轨道与无噪轨道一致、即与地址无关的公共末态（uncompute 后回到全零/全 $W$ 之前的某一确定配置），仅范数被标量修正。$\square$

**与 Wang 等人命题的关系。** [1, v2 Eq. (6)] 断言所有可靠分支共享末态 $|i\rangle|d_i\rangle|Q_0\rangle$。定理 3 是其 qubit 推广：共享性仍然成立（$|\tilde Q_0\rangle$ 与地址无关），但数据寄存器从确定值 $|j\oplus d_i\rangle$ 替换为**已知的二分量积态** $\bigotimes_t(\alpha|b_{\mathrm{out},t}\rangle+\beta|\overline{b_{\mathrm{out},t}}\rangle)$。这是两种编码在预测层面的全部差别。

### 4.5 误差结构：范数流失与相干泄漏

定理 3 的末态可按 $\gamma$ 的阶数分解。记单比特（$d=2n$ 层）：

- **正确振幅** $\alpha_{2n}=\frac{1+(1-\gamma)^n}{2}=1-\frac{n\gamma}{2}+\mathcal{O}(\gamma^2)$；
- **泄漏振幅** $\beta_{2n}=\frac{1-(1-\gamma)^n}{2}=\frac{n\gamma}{2}+\mathcal{O}(\gamma^2)$。

两个概念必须区分：

1. **范数流失（一阶，非误差）。** 单比特无跳变概率 $\alpha^2+\beta^2=\frac{1+a^{4n}}{2}=1-n\gamma+\mathcal{O}(\gamma^2)$。丢失的 $\approx n\gamma$ 是跳变轨迹的概率权重，由蒙特卡洛框架中 bad 分支的显式演化携带。good 分支的未归一化范数收缩**不是输出错误**。
2. **相干泄漏（二阶，真误差）。** 在 good 分支内部做条件归一化后，单数据比特的错误概率为
$$
\frac{\beta_{2n}^2}{\alpha_{2n}^2+\beta_{2n}^2}=\frac{n^2\gamma^2}{4}+\mathcal{O}(\gamma^3).
$$

**结论：good 分支的输出错误对 $\gamma$ 是二阶的；全部一阶错误都集中在 bad 分支（跳变与退极化噪声点）上。** 这解释了为什么"剪枝 + 闭式预测"在 qubit 编码下不损失一阶精度，也与信道平均结果自洽：单数据比特经完整阻尼信道后的错误概率为 $\frac{1-a^{2n}}{2}\approx\frac{n\gamma}{2}$，其中一阶项 $=$ 跳变概率 $\times\frac12$（跳变后输出随机），二阶余项正是 good 分支的相干泄漏。

该性质对 error filtration 类应用 [1, §IV；4] 是好消息：若结合对跳变的隐式后选（good-only 轨迹即"无跳变"假设下的输出），残余误差为 $\mathcal{O}(n^2\gamma^2)$ 每数据比特。

### 4.6 跳变概率采样中 good 分支的解析贡献

$Damp\_Full$ 噪声点处的准测量概率需汇总**所有**分支（含不演化的 good 分支）在出错比特上的占据权重。两点观察使 qubit 方案可以保持解析折算：

1. **数据部分输入无关。** X 基传输下，每个驻留数据比特对任一途经位置的平均占据概率恒为 $1/2$，与输入 $j$、存储 $d$ 无关（$\pm$ 相位不影响概率）。故参考 good 分支的数据部分跳变概率对全体 good 分支**精确成立**，无需逐分支修正。
2. **地址部分按相对乘子折算。** 与 qutrit 版相同：$\mathrm{prob}_{\mathrm{damp}}^{(i)}=\mathrm{prob}_{\mathrm{damp}}^{(\mathrm{ref})}\times\mathrm{relative\_multiplier}_i$（实现位置：`qram_circuit_qubit.h:496-519`）。

因此引入本文的数据总线闭式后，跳变采样的全局一致性不被破坏。

## 5. 算法

### 5.1 伪代码

```
算法 1：qubit 架构 (n,k)-QRAM 含噪快速模拟（单条轨迹）
输入：稀疏输入态 Σ_j c_{i,j} |i⟩|j⟩；存储表 d；噪声率 (ε_dep, γ)；a = √(1−γ)
输出：含噪输出稀疏态（未归一化），或末态采样

1  噪声历史预采样：对 t = 1..6n+2k，按活动子树规模 2(2^{L(t)}−1) 二项采样错误数、
   均匀采样位置；每个噪声点合并进 bad 地址区间并集 R：
   Damp_Full → 节点叶子子树（阻尼跳变不产生路径外残留）；
   Depolarizing 的 X/Y 分量 → 构型家族分歧判据（get_bad_range_qubit：
   左孩子取 subtree(parent(v))，右孩子取 subtree(v)，根/根孩子特判）。
2  分支划分：地址 i ∈ R ⇒ bad 分支；否则 good 分支，首个 good 记为参考分支 ref。
3  真实演化：仅对 bad 分支 ∪ {ref} 逐时间片执行：
   CopyIn_0 前作用 H；CopyOut_{k−1} 后作用 H（分支内 2^k 分裂 + try_merge 合并）；
   每步末 Damp_Common（振幅 × a^{#激发}）；Damp_Full 处按 §4.6 折算概率并采样投影。
4  good 分支解析预测：对每个 good 分支 (i, j)：
   a) 地址因子：c_addr(i) 按 §4.3 计数，相对权重 r_i = a^{2[c_addr(i)−c_addr(ref)]}；
   b) 数据因子：b_out = j ⊕ d_i；对 w = 0..k 的所有错误图样 c，
      A(c) = α^{k−|c⊕b_out|} β^{|c⊕b_out|}，α=(1+a^{2n})/2，β=(1−a^{2n})/2；
   c) 树末态：复制 ref 的树末态。
5  重构输出：每个 good 分支向输出稀疏态写入 2^k 个分量
      |i⟩|c⟩，振幅 = c_{i,j} × √(r_i) × A(c) × (ref 树末态振幅)；
   bad 分支按演化结果写入；合并同项、剔除 |振幅|²<ε 分量。
6  末态采样（可选）：按未归一化权重轮盘赌选定末态后归一化。
```

### 5.2 正确性

- 步骤 1–3 与 qutrit 版相同，正确性由 (P1)(P3) 与轨迹法给出 [1]。
- 步骤 4 的正确性即定理 3；其中窗口计数 $d_t=2n$ 的端点约定（$\pm1$）对全部 good 分支一致，且与参考分支真实演化中的逐步 `Damp_Common` 计数自校准——相对权重 $r_i$ 不含该约定，绝对因子 $(1+a^{2n})/2$ 的标定可通过与参考分支的 $2^k$ 分量比对一次性完成。
- 步骤 5 中 good 分支的 $2^k$ 展开是精确恒等式（定理 2），不是近似；与 qutrit 版"乘 $\sqrt{\mathrm{multiplier}}$ + 修正 bus 输出"（`SparQ/src/qram.cpp:100-170`）相比，仅多了按 $A(c)$ 分配的展开循环。

### 5.3 复杂度

设输入分支数 $d$、bad 分支数 $B$（期望 $\mathbb{E}[B]=\mathcal{O}(n^2p)\,d$ 量级，见 [1, v1 Eq. (6)]）：

| 项目 | qutrit 版 | 本文 qubit 版 |
|---|---|---|
| 真实演化分支数 | $B+1$ | $B+1$ |
| 每 good 分支预测成本 | $\mathcal{O}(n+k)$（标量乘子） | $\mathcal{O}(n+k)$（计数）$+\,\mathcal{O}(k\,2^k)$（写出分量） |
| 每 good 分支内存 | $\mathcal{O}(n)$ | $\mathcal{O}(n+k)$（积态可惰性存储） |
| 总复杂度 | $\mathcal{O}(d+p\cdot\mathrm{poly}(n))$ | $\mathcal{O}(d\cdot 2^k+p\cdot\mathrm{poly}(n))$ |

$2^k$ 因子是 qubit 编码的内在输出宽度（每个数据比特携带一个二分量叠加），并非算法低效：无噪 qubit-QRAM 的输出数据寄存器本来就是 $k$ 比特确定值，含噪后其支持集天然扩到 $2^k$。对 $k$ 较大的场景，可利用积态结构惰性展开（仅在采样/重构时按 Hamming 权重公式逐分量生成，期望每分量 $\mathcal{O}(k)$），或按 $\beta/\alpha$ 截断到 $w\le w_{\max}$（误差 $\mathcal{O}((n\gamma)^{w_{\max}+1})$）。实验中典型的 $k=1\sim3$ 下该因子可忽略。

## 6. 与 qutrit 方案的对照

| 方面 | qutrit 方案 | qubit 方案（本文） |
|---|---|---|
| 空闲态 | $\|W\rangle$ 是 $K_0$ 不动点 | 无；路由 $\|1\rangle$ 全程暴露 |
| 数据取数 | CNOT（计算基翻转） | CZ（相位编码），需 $H$ 换基 |
| 两片 H | 退化（仅 try_merge） | 真实分裂/合并 |
| 数据比特阻尼暴露 | 计算基，窗口依赖 $b_{\mathrm{in}},b_{\mathrm{out}}$ | X 基，固定窗口 $2n$，与比特值无关 |
| good 分支数据输出 | 确定值 $\|j\oplus d_i\rangle$ | 已知积态 $\bigotimes_t(\alpha\|b_{\mathrm{out},t}\rangle+\beta\|\overline{b_{\mathrm{out},t}}\rangle)$ |
| good 分支内误差 | 0（纯标量衰减） | $\mathcal{O}(n^2\gamma^2)$ 相干泄漏/比特 |
| 地址激发暴露 | $\mathcal{O}(t)$ 步/比特（传播式） | 全程 $\sim T$ 步/比特（驻留式） |
| X 型故障残留 | 冻结在故障节点（W 守卫），路径外构型同一 | 沿空闲祖先迁移，家族分歧至 subtree(parent) |
| 预测公式 | $a^{\Delta c}$ 标量 | $a^{\Delta c}$ 标量 × Hamming 权重公式 |
| 坏区间判据 | 子树包含（两通道通用） | 阻尼：子树包含；退极化 X：左孩子上溯（§3） |

## 7. 本仓库的实现缺口与路线图

对照当前代码，落地本算法需要：

1. ~~**qubit 噪声采样接通**~~（已完成）：`fill_bad_range` 已支持 qubit 架构（`time_step.cpp`），`arch2str` 补上 qubit 分支。
2. ~~**Full（不剪枝）ground truth**~~（已完成）：`qram_qubit::QRAMCircuit` 补齐 `set_input_uniform`/`set_input_random`、`run_full()`（= 全分支演化）、`sample_and_get_fidelity()`；修复了 `BranchGroup::get_prob_damp` 的概率折算索引、`sample_output_*` 的权重口径（`branch_probs × |amp|²`）、`Branch::run_damp_full` 缺失的跳变复位（投影后须把该比特置回 $|0\rangle$）、`Branch::get_fidelity` 的实现。实验入口：`Experiment_QRAM_FidelityV2 --architecture qubit`（full-only）。
3. **数据总线闭式预测（核心缺口，下一步）**：`QRAMLoad::_reconstruct`（`SparQ/src/qram.cpp:100-170`）与 `state_manipulator.cpp` 的 good 分支重构目前只做"标量乘子 + bus XOR 存储值"，需扩展为按定理 2 的 $2^k$ 分量写出（或惰性积态表示）。
4. **qubit 版 good 分支接线**：`TimeStep::get_multiplier_qubit` 已实现地址部分（`time_step.h:186-207`），但 `Branch::get_multiplier` 包装处于注释状态（`qram_branch_qubit.h:213-225`）；pruned 模式下 good 分支组的采样/归一化路径（`sample_output` 的 good 回退、`get_normalization_factor_with_damping` 的 good 项）需要与数据总线闭式联合重做。
5. **跳变概率的数据部分折算**：qubit 版 `run_damp_full`（`qram_circuit_qubit.h`）已包含参考分支折算框架，按 §4.6 补上数据部分（输入无关、恒 $1/2$ 占据）即可。
6. **验证**：对照 `run_full`（全演化，本步已备）与 `run_normal`（剪枝）在相同种子下的 `sample_and_get_fidelity` 一致性（沿用 `Experiments/QRAM/QRAMFidelityV2/QRAMSimulatorTest.cpp` 的 test1 框架，`--architecture qubit`），以及 good-only 模式（`QRAMLoadFast` 的 qubit 对应物）。
7. ~~**坏区间判据确认**~~（已完成）：`get_bad_range_qubit` 的"左孩子上溯父区间、右孩子取自身子树"逻辑经机制推导与 n=3 单错误注入验证为**构型家族分歧的正确包络**（node1 特判亦机制正确：根从不空闲），此前被误记为"疑似怪癖"。推导见 `qubit_error_propagation.md`。注意两点：纯阻尼通道可用更紧的纯子树判据；n≥4 的深层树上"沿空闲链多级爬升"是否越出 subtree(parent(v)) 未验证，剪枝实现时建议做一次 n=4/5 注入回归。

## 8. 结论

我们证明了 qubit 编码 bucket-brigade QRAM 在振幅阻尼噪声下的 good 分支依然是闭式可预测的：$H\to K_0^d\to H$ 结构对第 $t$ 个数据比特的作用等价于已知单比特映射 $N|b\rangle=\alpha|b\oplus d^{(t)}\rangle+\beta|\overline{b\oplus d^{(t)}}\rangle$，其中 $d=2n$ 为固定暴露窗口，$\alpha=(1+a^{2n})/2$，$\beta=(1-a^{2n})/2$；$k$ 比特输出振幅只依赖错误图样的 Hamming 权重。结合既有的地址标量乘子与子树包含剪枝，qubit-QRAM 的含噪模拟达到与 qutrit 版相同的复杂度标度，且 good 分支内误差仅为 $\mathcal{O}(n^2\gamma^2)$ 的相干泄漏。这为在本模拟器中补齐 qubit 架构的噪声快速模拟、并进而支撑 error filtration 等下游研究提供了完整的理论基础。

## 参考文献

1. Y.-J. Wang, T.-P. Sun, X.-N. Zhuang, X.-F. Xu, H.-Y. Liu, C. Xue, Y.-C. Wu, Z.-Y. Chen, G.-P. Guo, *Efficient Simulation of Quantum Random Access Memory*（v2: *Refined Criteria for QRAM Error Suppression via Efficient Large-Scale QRAM Simulator*）, Phys. Rev. Applied **25**, 044069; arXiv:2503.13832.
2. Z.-Y. Chen 等，$(n,k)$-QRAM 并行查询协议（bucket-brigade 查询线路与 qubit/qutrit 两种编码）, arXiv:2303.05207.
3. C. T. Hann, G. Lee, S. M. Girvin, L. Jiang, *Resilience of quantum random access memory to generic noise*, PRX Quantum **2**, 020311 (2021).
4. S. Lee 等，*Error filtration for quantum communication/quantum memory*（error filtration 方案）, Phys. Rev. Lett. **131**, 190601 (2023).
5. V. Giovannetti, S. Lloyd, L. Maccone, *Quantum Random Access Memory*, Phys. Rev. Lett. **100**, 160501 (2008).

---

### 附：关键记号表

| 记号 | 含义 |
|---|---|
| $n,k$ | 地址比特数、数据比特数；$N=2^n$ |
| $T=6n+2k$ | 总时间片数 |
| $\gamma,\ a=\sqrt{1-\gamma}$ | 单层阻尼强度、单步衰减因子 |
| $K_0,K_1$ | 振幅阻尼无跳变/跳变 Kraus 算子 |
| $\tau_{\mathrm{in}}(i),\tau_{\mathrm{out}}(i)$ | 数据比特 $i$ 换入/换出时间片，窗口 $d_i=2n$ |
| $\alpha_d,\beta_d$ | $(1\pm a^d)/2$，正确/泄漏振幅 |
| $c_{\mathrm{addr}}(i)$ | 地址 $i$ 的路由激发暴露计数 |
| $b_{\mathrm{out}}$ | $j\oplus d_i$，理想输出数据 |
| $w$ | 错误图样 $c\oplus b_{\mathrm{out}}$ 的 Hamming 权重 |
