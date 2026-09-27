# qubit 振幅阻尼的精确联合抽样

qubit 引擎现在对同一阻尼时间片只做**一次联合辅助掩码抽样**，随后批量作用 Kraus 算符，并统一归一化一次。保留候选点预采样和 good 分支预测，不要求对指数多个物理位点逐一计算布居数或 rescale。

本文取代旧文档中逐 `Damp_Full` 独立抽样、以及把原始轨迹范数称为 survival 的说明。此次实现针对 **qubit 编码**。qutrit 引擎仍保留历史抽样规则，其 faithful mirror 测试只能视为旧行为回归，不能认证标准阻尼通道。

## 物理通道与作用范围

每个树比特的标准通道为

$$K_0=|0\rangle\langle0|+\sqrt{1-\gamma}|1\rangle\langle1|,\qquad
K_1=\sqrt\gamma|0\rangle\langle1|,\quad 0\leq\gamma<1.$$

每个阻尼时间片对整树 $M=2(2^n-1)$ 个比特施加此通道，外部地址和总线不受噪声。Pauli 噪声仍采用调度器的活动前沿范围。独立电路基线明确使用相同放置规则。同一片内先执行抽中的 Pauli 操作，再执行连续的联合阻尼层：保留候选生成时的 RNG 顺序，但把候选操作记录移到 Pauli 之后。联合阻尼中穿插门或使用不一致的 gamma 会明确报错。

整树阻尼与既有 `Damp_Common` 和 good 分支的解析曝光计数一致。旧代码只在活动前沿抽 K1 候选，却对整树作用 K0，导致前沿以外的激发没有对应跳跃通道；根部地址复制早期甚至可能出现前沿为空而根部已有激发的情况。因此，扩大 qubit 阻尼候选范围是物理通道修正的一部分。

## 联合辅助抽样及证明

设片初完整归一化态为 $|\psi\rangle=\sum_xc_x|x\rangle$，$X(x)$ 是配置 $x$ 的树激发集合：

1. 预采样候选集合 $C$，各位点独立以概率 $\gamma$ 入选。现有 binomial 数目加均匀子集的分布可以保留。
2. 按 $|c_x|^2$ 抽一次辅助配置 $x$。
3. 确定实际跳跃集合 $J=C\cap X(x)$。
4. 对**原完整相干态**作用 $K_J=\prod_{q\in J}K_{1,q}\prod_{q\notin J}K_{0,q}$，然后统一归一化。

辅助配置只用于产生环境 Kraus 标签，绝不能用它替换模拟态，否则会破坏相干性。

$$\Pr(J)=\sum_{x:J\subseteq X(x)}|c_x|^2\gamma^{|J|}(1-\gamma)^{|X(x)|-|J|}
=\langle\psi|K_J^\dagger K_J|\psi\rangle.$$

所以归一化条件轨迹的平均严格等于完整 Kraus 通道。这是有限 gamma 下的恒等式，没有小时间步或单跳跃近似。不同位置的局域通道对易；顺序执行时的条件概率仍依赖此前结果。

实现中只抽辅助配置在候选集上的限制：用 `std::map<vector<size_t>, double>` 汇总 $C\cap X$ 的分布。无需构造辅助配置的其余部分。掩码按规范顺序排列，使 pruned/full 在相同随机数下选择相同区间。非空候选片消耗一个均匀随机数；候选为空时不消耗辅助随机数。

## 稀疏更新与预测分支

对每个存储分量的激发集合 $X$：若 $J\not\subseteq X$ 就将其投影掉；否则清除 $J$，振幅乘 $(\sqrt{1-\gamma})^{|X|-|J|}$。公共因子 $(\sqrt\gamma)^{|J|}$ 并入条件态的整体尺度。最后只做一次完整代表态的范数计算和整体归一化。该范数不能再当作额外的存活权重。

`damping_mask_weights` 包含显式组以及所有隐式 good 组。后者通过参照组在候选掩码上的**联合分布**，乘以输入权重和地址衰减因子计入。这要求比单点布居相同更强的不变量：预测组在候选集合上的联合分布与参照一致，仅整体存活权重不同。候选坏区间排除和共享树结构是其依据；多候选回归验证最终逐轨迹等价性。

`get_multiplier_qubit(gamma, step)` 表示本片之前的衰减；完成 K0 后、统一归一化前使用 `step+1`。只 rescale 显式组，隐式组通过参照同步获得相同尺度。参照湮灭时仍需在物化时清零对应 good 组。

输入语义必须明确：同一地址下不同 `branches` 的概率账本保留正交的输入列标签，`branch_probs` 不能表示不同总线输入列之间的相干交叉项。电路级物理验证采用每地址一个输入总线列的 data-loading 输入；单个 Branch 内的任意复叠加由局域通道测试覆盖。此修改不宣称已实现任意相干多列输入 API。

## 旧方法的问题

旧代码外层候选概率为 $\gamma$，候选内接受概率为 $p_1$，**没有重复乘 gamma**。把第二次 C++ 抽样描述为 $\gamma p_1$，混淆了无条件跳跃概率和候选内条件概率。

实际错误是联合分布：跳跃时立即投影；拒绝跳跃时不更新，直到片末才统一作用 K0。后续布居因此缺少此前无跳跃带来的条件信息。记 $q_A=\langle n_A\rangle$、$q_B=\langle n_B\rangle$、$q_{AB}=\langle n_An_B\rangle$，则

$$q_{B\mid A0}=\frac{q_B-\gamma q_{AB}}{1-\gamma q_A}.$$

对 Bell 态，标准零次、一次、两次跳跃概率分别为 $1-\gamma+\gamma^2/2$、$\gamma-\gamma^2$、$\gamma^2/2$；旧方法为 $1-\gamma+\gamma^2/4$、$\gamma-3\gamma^2/4$、$\gamma^2/2$。gamma=0.2 时，一次跳跃被从 0.16 算成 0.17。结束后逐轨迹归一化也修复不了已抽错的跳跃分布。

另有两个不能靠内部对照掩盖的问题：

- 整树 K0 与仅活动前沿 K1 不组成同一保迹通道。
- 已按结果概率抽出的轨迹不能再用原始范数重复加权；旧的状态依赖 faithful 密度矩阵 mirror 也不是固定线性 CPTP 通道。它与旧引擎一致、或 pruned 与 full 一致，都不能代替标准通道验证。

## 复杂度和兼容性

记显式分量数为 $S$、每分量稀疏激发上限为 $h$、候选数为 $c$、预测组数为 $G$。提取掩码时遍历激发集合并在有序候选中二分查找，不扫描所有 $M$ 个树位点。还需计入不同掩码的 map 聚合成本、一次稀疏投影/衰减、代表态范数与 rescale，以及对 $G$ 个预测权重的汇总和解析曝光计算。

这不等于总体多项式复杂度证明：显式候选表每片期望大小仍为 $M\gamma$，态支持大小、总线宽度和预测维护也可能增长。

候选域、RNG 消耗、轨迹归一化及 seed 到历史的映射均已改变。所有旧的非零阻尼数据和计时图都需要重跑。新的 pruned/full 必须逐 seed 一致，但不应要求新旧采样器给出同一历史。保留的公开 `run_damp_full` 只是一候选投影兼容接口；连续独立调用不能替代 `run_damping_layer`。

## 验证与复现

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DCACHED_REGISTER_SIZE=8
cmake --build build -j
ctest --test-dir build --output-on-failure
./build/bin/CorrectnessTest
./build/bin/ExecutabilityTest
./build/bin/Experiment_QRAM_QubitPaperExactTest --perf-shard 0 1
```

- `QubitJointDampingChannel`：1–4 比特，gamma={0,1e-5,0.2,0.7,0.95}，基态、全激发、GHZ、随机复态，共 80 组完整枚举；比较独立稠密逐比特 Kraus 通道的全部密度矩阵元。另含 60,000 条 Bell 抽样和逐片范数检查。
- 两套 exactness 电池：比较树测量前后完整加权复分量、地址概率、保真度、RNG、候选历史和实际联合跳跃标签。固定历史覆盖参照死亡、存活及电路复用。
- `verify_noisy_simulation`：9 比特编码电路（n=2,k=1），共享调度输入但使用独立矩阵门和通道后端。包括逐历史回放、每组 2,000 条轨迹的 pure depolarizing / pure damping / mixed 比较，并测试 gamma=0.2。严格检查标准通道和轨迹均值迹为 1，完整密度矩阵 Frobenius 误差与 Monte Carlo 估计误差比较，同时报告 TVD 和保真度。qutrit 旧 mirror 单独标为 legacy。

初始实测：局域密度矩阵最大误差约 $4.45\times10^{-16}$；Bell 一次跳跃频率 0.15875（精确值 0.16）；电路输出 TVD 为 0.0017–0.0133，完整密度矩阵误差为 0.0089–0.0165，与 Monte Carlo 误差相容。有限回归通过不能替代所有共享参照不变量的数学证明。

外部 uniqc/QuTiP 对照另通过 pure damping 与 mixed noise 两组测试（各 2,000 条轨迹、6 条固定历史回放）。新导出记录 `joint_auxiliary_whole_tree_v1`，driver 自动采用标准 Kraus 通道并在回放时逐片归一化。

最终 pruned/full 验证：155 例主电池与 120 例分通道电池全部通过；主电池触发 178 次跳跃，加权复分量最大差 6.67e-16，保真度最大差 1.34e-15。外部 pure damping 导出须显式设置 `--depolarizing 0`，因为 CLI 默认 Pauli 率并非零。
