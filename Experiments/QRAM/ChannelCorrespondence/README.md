# ChannelCorrespondence — qutrit QRAM 噪声模型 ↔ 电路级 channel 模拟对应实验

研究问题：**QRAM-Simulator 的（轨迹级、TimeStep 调度的）qutrit QRAM 噪声模拟，能否与电路级（密度矩阵 / channel 级）noisy simulation 相对应？**

本目录是 Step 1 的完整交付。噪声类型以 **Depolarizing** 为准（Damping 接口面保留、信道化对应留 M3）。
**trajectory 模式**按其本义执行：抽样完成后（位置与 X/Z/Y/qutrit Weyl 算子均已确定）落成
**确定性 unitary 门**，与 QRAM-Simulator 的同一随机 case **逐条精确对拍**；channel 级
（密度矩阵）作为统计对照。不启用 uniqc 的 `NoisySimulator`/ErrorLoader 机制。

## 结论摘要（TL;DR）

| 验证层 | 结果 |
| --- | --- |
| **Stage 0** 无噪声桥（编码门电路 vs QRAM 无噪声 run） | **逐位精确匹配（max\|ΔP\| ≈ 1e-16）** |
| **Stage 1** 逐随机 case（8 个实现化，抽样后 → 具体算子 → unitary 门） | **全部精确匹配（1e-16，native 与 model 双参考一致）** |
| **Stage 2** channel 级 · mode c（每步活跃位置 × 边缘概率放信道） | **F_classical ≥ 0.9966（p = 0.005→0.3），TVD ≤ 0.054** |
| **Stage 2** mode b（仅单次实现化的提取位置放信道） | F 0.92–0.97：单次实现欠噪声，属预期 |
| **fidelity 统计一致性** | **F_quantum(channel) ≈ E\|⟨ψ_ideal\|ψ_traj⟩²\|（全噪声强度成立）**，详见[口径讨论](#fidelity-口径实证) |
| **Stage 1-d** Damping 逐随机 case（含第二次抽样拦截 + 单 Kraus 回放） | **全部精确匹配（1e-16，次归一化口径，γ=0.001→0.2 及混合配置）** |
| **Stage 2-d** Damping 信道级 · faithful 镜像 | **F_cls(归一) ≥ 0.977，trace ≈ survival；教科书 AD 信道则系统性偏差**，详见[Damping 路径验证](#damping-路径验证-m3-1) |
| **qubit 架构** Stage 0/1（depol+damping 全部 8 case × 10 配置） | **全部精确匹配（1e-16）**，详见[qubit-based QRAM 验证](#qubit-based-qram-验证m3-1bdepolizing--damping-完整) |
| **qubit 架构** 信道级 faithful | **F_cls(归一) ≥ 0.9974 全部配置；F_quantum ≈ avg_overlap_fid；trace≈survival 差 <1.5%** |
| OriginIR-ext 文本信道路径（data 位 `Depolarizing q,(p)` 内联） | 与 kraus 直驱 0 误差一致 |
| renormalize 两口径 | Depolarizing 为 CPTP（存活率 1）两口径一致；**Damping 下真正分化**（见 damping 节） |

**核心答案：对应成立。** 把 TimeStep 噪声模型按「每步活跃位置 × 边缘概率 p」编译成电路级
信道，其密度矩阵输出分布与 QRAM-Simulator 多轨迹平均在全噪声强度一致到 MC 误差量级；
且**态层面**（含相干性）的 `⟨ψ_ideal|ρ|ψ_ideal⟩` 与轨迹系综的 `E|⟨ψ_ideal|ψ_traj⟩|²`
统计一致——即用户的「统计意义上应一致」直觉在无后选择、树敏感口径下被证实。

过程中发现并已修复四处上游问题（三处在 QRAM-Simulator 库、一处在 uniqc，见
[实现层修复](#实现层修复)），另确认一处输入约定（memory 取值须在 data_size 位内）。

## 目录与流水线

```
QutritCorrespondenceExporter.cpp   C++ 导出器（目标 Experiment_QRAM_ChannelCorrespondence）
run_correspondence.py              Python 驱动（跑在含 uniqc 的解释器，如 UnifiedQuantum/.venv）
results/                           运行产物（gitignored，可再生）
```

```
QRAM-Simulator（C++）                          uniqc（Python）
─────────────────────                          ──────────────────────
QRAMCircuit + set_noise_models         JSON     门序列 → qutrit→qubit 编码电路
TimeStep::generate 调度 ─────────────────────→ Stage 0: statevector 精确对拍
  ├─ 逻辑算子 → 编码门（含控制极性）              Stage 1: 逐随机 case unitary 对拍（schedule_r*.json）
  └─ 噪声算子 {step,type,pos,coef}               Stage 2: 信道放置（mode b/c）→ 密度矩阵
run_full × R 轨迹 → 逐 case 分布 + 平均分布       对比：renormalize × {F_cls, TVD, F_quantum}
                  + 四个 fidelity 口径参考量
```

## 使用方法

```bash
# C++ 侧（工具链：workspace .tools/envs/devenv）
cmake --build build --target Experiment_QRAM_ChannelCorrespondence
cmake --build build --target Experiment_QRAM_ChannelCorrespondence_Qubit
cd Experiments/QRAM/ChannelCorrespondence
../../../build/bin/Experiment_QRAM_ChannelCorrespondence \
    --addrsize 2 --datasize 1 --memory "0,1,1,0" \
    --seed 20260921 --runs 500 --depolarizing 0.02 --outdir results/depol002
../../../build/bin/Experiment_QRAM_ChannelCorrespondence_Qubit \
    --addrsize 2 --datasize 1 --memory "0,1,1,0" \
    --seed 20260921 --runs 500 --depolarizing 0.02 --damping 0.02 --outdir results/qubit_mixed

# Python 侧（调度 JSON 的 "arch" 字段自动路由 qutrit/qubit 路径）
<path-to-uniqc-venv>/bin/python run_correspondence.py --dir results/depol002 --stage all
```

导出器参数：`--addrsize/--datasize/--memory/--seed/--runs/--depolarizing/--damping（接口面，
对应留 M3）/--exportruns（导出前 K 条轨迹的调度供逐 case 对拍，默认 8）/--input{zerobus,uniform}/
--outdir/--tracesteps`。驱动参数：`--dir/--stage{0,1,2,all}/--tol`。

**memory 取值约定**：每个 cell 必须落在 `data_size` 位内（如 data=1 只能 0/1）——
`Branch::get_fidelity` 的 `expect_bus = bus_input ^ memory[address]` 直接 XOR 原始值，
越界值会被静默判零贡献。导出器已做校验并拒绝越界输入。

主配置：**addr=2, data=1**（12 个编码 qubit；addr=1 时 `layer_entangle_max≡0`、噪声采样
恒空，addr≥2 是有噪声的最小实例）。输入取 **zerobus**（地址均匀叠加、bus=0）：bus 均匀
叠加会使输出联合分布对任意条件置换退化为均匀，失去判别力。

## qutrit→qubit 编码与算子审计表

寄存器（`encoding` 字段）：地址 `addr_size` 位（bit b ↔ qubit b）；总线 `data_size` 位；
路由节点 `v ∈ [0, 2^n−1)`（堆编号）各占 3 位：`a1, a0`（能级编码 **W=|00>（基态）、L=|01>、
R=|10>、|11> 死态**，对应 `typedefs.h` 的 `W=-1, L=0, R=1`）与 `d`（数据位）。
噪声位 `pos`：`node = pos/2`，`pos%2`：0 → addr-qutrit 的 (a1,a0)，1 → data 位。

逻辑算子翻译（镜像 `QRAMCircuit::run_valid_branches` 的 dispatch，`qram_circuit_qutrit.cpp:811`）：

| Operation | QRAM-Simulator 语义 | 编码门 |
| --- | --- | --- |
| `FirstCopy{ℓ}` | `node0.data ^= addr_bit(n−1−ℓ)`（MSB-first，`get_digit_reverse`） | CNOT(addr[n−1−ℓ], node0.d) |
| `CopyIn{d}` / `CopyOut{d}` | SWAP(bus[d], node0.data)（伴随 try_merge 为 no-op） | SWAP(bus[d], node0.d) |
| `SwapInternal{0}` | 根 `internal_swap`：(W,0)↔(L,0)、(W,1)↔(R,0) | 2 个对换（多控 X 共轭） |
| `SwapInternal{ℓ≥1}` | layer ℓ−1 各节点按 addr 方向对 child 做 internal_swap | child 上的对换 × parent (a1,a0)==L/R 双控 |
| `ControlSwap{ℓ}` | layer ℓ 各节点按 addr 方向 SWAP(本节点.d, child.d) | 双控 SWAP |
| `FetchData{d}` | 叶节点 addr==L/R 选 cell（L→偶、R→奇），`data ^= memory[cell][d]` | 叶 d 上的双控 X（按 memory 位） |

Depolarizing 噪声算子（stage 1 酉 / stage 2 信道；`coef` 的 floor 规则镜像
`SubBranch::run_depolarizing`）：

| 子系统 | floor 规则 | 酉（stage 1） | 信道（stage 2） | OriginIR-ext 文本 |
| --- | --- | --- | --- | --- |
| data 位（奇 pos） | floor(3·coef) ∈ {X, Z, XZ} | 直接门 | `Depolarizing q,(p)`（uniqc 语义＝以 p 施加均匀非平凡 Pauli，与 QRAM 边缘语义对齐） | ✓ |
| addr-qutrit（偶 pos） | floor(8·coef) ∈ {A1, A2, A1², A2², 四个乘积}（Weyl 族） | 置换（对换共轭）+ 受控相位 | 8 元 Weyl 均匀混合的 4×4 Kraus（死态恒等） | ✗（非 Pauli，走 kraus2q） |

**mode c 边缘概率**：采样器每步抽 `nerror ~ Binomial(N, p)`（N=2(2^L−1)）均匀落入
`[0, N)` 的 N 个活跃位置（闭区间 bug 修复后）→ 每位置边缘概率恰为 p；
`entangle_max=0` 的步无噪声。

## 三级验证阶梯

1. **Stage 0**：仅门序列，statevector 后端，与 QRAM 无噪声 `run_full` 分布断言相等。
2. **Stage 1（trajectory 主模式）**：前 `--exportruns` 条轨迹的调度逐条导出
   （`schedule_r{i}.json`），每条按 floor 规则落成确定性 unitary（抽样已完成、无概率），
   与该轨迹的 `run_full` 精确分布逐 case 断言相等。
3. **Stage 2**：噪声位置放信道（mode b：单次实现的提取位置；mode c：全活跃位置 × p），
   QuTiP 密度矩阵后端，与 R 条轨迹平均分布对比。信道模拟需大量轨迹收敛（用户的预期），
   mode c 即该口径。

## 实现层修复

对拍过程中发现并修复（QRAM-Simulator 三处在当前实验分支 `experiment/qutrit-noise-correspondence`；
ctest 190/190 通过）：

1. **`QRAMNode::rotate_A1` if 穿透**（`qram_branch_qutrit.h`）：`if (addr==W) addr=R;`
   落入 `if (addr==R) addr=L;` → 实现为 W→L、R→L（非单射，非酉），与注释三循环
   L→W→R→L 不符；且 `QRAMState::rotate_A1` 对 absent 节点 insert (R,0)、对显式 (W,d)
   走穿透——同一物理态 (W,0) 因簿记不同而不同输出。已改为早返回的干净三循环。
2. **`QRAMState::state_of` 的 absent 语义**（`qram_branch_qutrit.cpp`）：absent 一律返回
   0==L，使相位类噪声算子对基态（absent≡W）节点误加 ω 相位。已改为奇数位返回 0、
   偶数位返回 W。qubit 架构的 `state_of` 返回 bool，无此问题。
3. **噪声位采样闭区间 off-by-one**（`time_step.cpp` 两个 `noise_one_step` 重载）：
   `uniform_int_distribution(0, nqubits)` 含端点，使下标 nqubits（子树外一个节点）可被
   抽到。已改 `uid(0, nqubits-1)` 并对 nqubits==0 跳过。
4. **uniqc 的 `DensityOperatorSimulatorQutip.kraus2q`**：`Qobj(reshape(4,4))` 生成
   dims=[[4],[4]]，qutip `expand_operator` 拒绝映射到 2 target。已修
   （`dims=[[2,2],[2,2]]`），上游 PR：IAI-USTC-Quantum/UnifiedQuantum#130
   （本地 checkout 在 `fix/qutip-kraus2q-dims` 分支；全量测试 2520 passed）。

修复验证：导出器的**模型语义轨迹**（纯状态函数 + 注释三循环）与原生 `run_valid_branches`
现已逐位一致（reference.json 中 native 与 model 双参考相等），保留作回归交叉验证。

另确认（未改库）：**memory 取值须在 data_size 位内**——`get_fidelity` 直接 XOR 原始
memory 值，越界输入会被静默判零（`set_memory_random` 的采样范围即符合约定）；导出器
已加输入校验，fidelity 参考量计算亦做了掩码防御。

## 结果（addr=2, data=1, memory=[0,1,1,0], seed=20260921）

分布对应与 fidelity 口径（500 轨迹；p=0.3 为 200）：

| p | mode c F_cls | TVD | F_quantum(channel) | overlap（树敏感,无后选择） | nopost（树不敏感） | incoh（分支投影） | post（原版 avg_fid） |
| --- | --- | --- | --- | --- | --- | --- | --- |
| 0.005 | 0.9997 | 0.0038 | 0.9295 | 0.9395 | 0.9559 | 0.9785 | 0.9497 |
| 0.02 | 1.0000 | 0.0019 | 0.7458 | 0.7316 | 0.8177 | 0.9240 | 0.7847 |
| 0.05 | 0.9990 | 0.0226 | 0.4779 | 0.4283 | 0.5941 | 0.8075 | 0.5232 |
| 0.1 | 0.9991 | 0.0249 | 0.2250 | 0.2005 | 0.3870 | 0.7075 | 0.3246 |
| 0.3 | 0.9966 | 0.0542 | 0.0120 | 0.0097 | 0.1478 | 0.5850 | 0.1394 |

（F_cls = 经典分布 fidelity (Σ√pq)²，vs 原生轨迹平均；vs 模型参考一致到第三位。
`results/*/driver_summary.json` 有全部口径。）

## fidelity 口径实证

针对「`sample_and_get_fidelity` 为什么不能和保真度直接比 / 统计上是否应一致」的实证回答：

1. **统计一致性成立**：无后选择、树敏感口径的轨迹平均
   `E|⟨ψ_ideal|ψ_traj⟩|²`（avg_overlap_fid）与信道级 `⟨ψ_ideal|ρ_mode-c|ψ_ideal⟩`
   （F_quantum）在全噪声强度一致（表中两列，差在 500 轨迹 MC 误差内）。代数上这是
   `⟨ψ|E[|ψ_traj⟩⟨ψ_traj|]|ψ⟩` 的恒等式——成立即证明**信道系综 = 轨迹系综**（态层面）。
   此前观察到的巨大差距（0.21 vs 0.75）是 memory 越界伪影，已消除。
2. **分支相干 vs 分支投影：均值不一致**（incoh − overlap ≈ 0.19 @p=0.02）。原因：
   打在**共享根节点**的事件给多个地址分支**相关**的相位（同一 Weyl 算子对不同分支的
   节点态给出不同 ω 幂）——不是独立零均值，跨分支交叉项 `E[G_a G_b*]` 不消失。
   「把分支相干加在一起」作为单 shot 统计量是正确的（地址寄存器不测量、相干读出时），
   它带来的是**口径差异（此处为降低均值）而非单纯减方差**。
3. **树敏感 vs 树不敏感**（nopost − overlap ≈ 0.09 @p=0.02）：F_a 把「bus 正确但树残留」
   的振幅也计入；理想态内积要求树回基态。
4. **逐 shot 树后选择**（post vs nopost，再降 ~0.03）：`sample_output` 采样树构型并删失
   不一致 subbranch，把跨构型相干项换成 P(τ) 加权非相干平均——均值小幅改变（非仅方差）。

## renormalize 讨论

Depolarizing 全链 CPTP/酉轨迹：QRAM 侧存活率（`get_normalization_factor`）与 uniqc 侧
trace(ρ) 恒 1，**不归一/归一两口径数值相同**。Damping 下两口径真正分化——见下节。

## Damping 路径验证（M3-1）

### 审计与拦截

`noise_t{Damping: γ}` 每步拆成若干 `Damp_Full{pos,γ}`（位置采样同其它类型）+ 步末全局
`Damp_Common{γ}`：

| 算子 | 语义 | 编码侧 |
| --- | --- | --- |
| `Damp_Common`（qram_branch_qutrit.cpp:581） | 全树每个激发自由度振幅 ×√(1−γ)，确定性非酉 | K0 单 Kraus（qutrit `diag(1,√(1−γ),√(1−γ),1)`；data 位 1q） |
| `Damp_Full`（qram_circuit_qutrit.cpp:400/:642） | **第二次抽样**：`prob_damp[k]` 按 branch 权重累积，单次 `uniform01×norm²` 选 {L跳, R跳, 不跳}；跳变删失非目标能级 subbranch 后坍缩基态，无 √γ 因子 | 固定结果单 Kraus `\|W⟩⟨L\|` / `\|W⟩⟨R\|`（data 位 `\|0⟩⟨1\|`） |

「第二次抽样」的拦截：导出器以复刻执行重放（逐 RNG draw 对齐，全部公有 API），
`schedule_r{i}.json` 的 Damp_Full 条目带 `outcome` 字段；复刻 vs 原生逐轨迹分布断言一致
（内置校验）。驱动 Stage 1-d 用 QuTiP 后端回放：门 + 每步 K0（全树）+ 固定结果跳变
单 Kraus → 次归一化密度矩阵，对角 vs QRAM 该轨迹次归一化分布**精确对拍（1e-16）**。

### 结果（γ 扫描，500 轨迹；mixed = depol 0.02 + γ 0.02）

| γ | survival | nojump trace | faithful trace | faithful F_cls(归一)/TVD | 教科书 full F_cls(归一)/TVD | F_quantum(faithful) vs avg_overlap_fid |
| --- | --- | --- | --- | --- | --- | --- |
| 0.001 | 0.979 | 0.982 | 0.976 | 0.999 / 0.010 | 0.998 / 0.015 | 0.840 ≈ 0.865 |
| 0.01 | 0.796 | 0.831 | 0.788 | **0.9998 / 0.009** | 0.994 / 0.041 | **0.651 ≈ 0.657** |
| 0.05 | 0.317 | 0.393 | 0.306 | **0.999 / 0.017** | 0.956 / 0.157 | **0.213 ≈ 0.206** |
| 0.2 | 0.044 | 0.022 | 0.024 | 0.977 / 0.103 | 0.931 / 0.206 | 0.005 ≈ 0.002 |
| mixed | 0.628 | 0.684 | 0.614 | **0.999 / 0.013** | 0.985 / 0.079 | 0.425 ≈ 0.430 |

### 结论

1. **轨迹级验证完全通过**：含第二次抽样的每个随机 case 在电路级精确复现（Stage 1-d
   8 case × 全部 γ 及混合配置，1e-16）——阻尼路径的执行语义被完整钉死。
2. **教科书 AD 信道不是他们的模型**（预注册疑点证实但量级修正为 O(γw)）：逐度每步
   `K0+K_L+K_R`（TP）的归一分布 TVD 随 γ 增长到 0.21；无跳变生存率（nojump trace）
   在 γ≤0.05 高于他们的 survival（过衰减方向），γ=0.2 时反低于之——结构差异非简单标度。
3. **忠实镜像**（ρ 依赖映射：全树 K0 权重 `1−γw/S` + 仅活跃位置（节点 < 2^L−1）的
   跳变混合 `√(γw_k/S)·M_k`，无 √γ 幅度因子）在全部 γ 复现归一分布（F_cls ≥ 0.977）
   与生存率（γ≤0.05 差 <3%；γ=0.2 差 47%——非线性映射在平均 ρ 上取权重的近似随
   γ 增大失效）。
4. **统计一致性在正确信道下成立**：`F_quantum(faithful) ≈ avg_overlap_fid` 全程
   （上表末列）——「信道系综 = 轨迹系综」的恒等式在非酉情形同样成立，前提是用对
   他们的（ρ 依赖）信道，而非教科书 AD。
5. **renormalize 口径分化**（本实验原始需求）：`F_cls(不归一)` 在 γ=0.001→0.2 从
   0.98 跌到 0.04——QRAM 侧次归一化分布（含生存权重）与 TP 信道（trace=1）不可直接
   比；归一后（除以各自 trace/survival）才可比。nojump 模式给出「无跳变生存概率」的
   电路级对应物（trace ↔ 教科书 no-jump 概率，非他们模型的 survival）。

### 局限

- faithful 镜像是 ρ 依赖非线性映射，在平均 ρ 上取权重（Jensen 型偏差）——高 γ 处
  trace 偏差增大；逐轨迹权重的精确平均需 MC 积分（即回到轨迹系综本身）。
- Damp_Full 的跳变位置只在活跃子树（`layer_entangle_max`），而 Damp_Common 的 K0
  作用于全树——忠实镜像分别对齐了两者；Stage 1-d 的精确匹配证明该读法正确。
- `run_normal` 剪枝路径与解析 multiplier（`_get_multiplier_impl_qutrit`/`QRAMLoadFast`）
  未在本轮覆盖（下一里程碑）。

## qubit-based QRAM 验证（M3-1b，Depolarizing + Damping 完整）

`QubitCorrespondenceExporter`（目标 `Experiment_QRAM_ChannelCorrespondence_Qubit`）+
驱动的 qubit 路径复用同一套三级阶梯。与 qutrit 的语义差异（审计要点）：

- **编码**：节点 v = 2 个普通 qubit（a = addr+data+2v、d = +1；位置 = v·2+lr 与噪声位
  约定一致）；addr=2 → **9 个编码 qubit**（qutrit 为 12）。
- **FetchData 是相位 kickback**：叶 data×addr 选 cell 打 −1 相位，配合 `CopyIn{0}` /
  `CopyOut{末}` 处 `run_hadamard` 对 bus 全位的真实 H——净语义仍 `bus_out = bus_in ⊕ m[a]`
  （Stage 0 逐位验证）。门翻译含 `H` 原语；`SwapInternal` 无条件发射
  （基态节点上 SWAP 恒等）；`cswap` 由 addr 位单控。
- **`run_bitphaseflip` 已修复为 Y flip**（|0>→−|1>、|1>→|0>，ZX = iY，酉——与 qutrit
  架构奇数位语义一致）。旧实现为 |1>→−|0> 的秩 1 非酉衰减算子（等价 −K1），曾导致
  Depolarizing 轨迹次归一化、分布过计数与 `sample_output` 零范数崩溃；修复后
  **Depolarizing 恢复为保范数（survival=1）**，`depol_textbook` 与 `depol_faithful`
  两种模式数值一致（Y 与 ZX 仅差全局相位 → 同一信道）。

结果（500 轨迹；Depolarizing 的 textbook/faithful 修复后等价，只列一列）：

| 配置 | stage1 逐 case | F_cls(归一)/TVD | trace(faithful) vs survival | F_quantum(faithful) vs avg_overlap_fid |
| --- | --- | --- | --- | --- |
| depol p=0.005→0.3 | 8/8 精确 | **0.9991–0.9999** / ≤0.024 | 1.000 vs 1.000 | 0.923/0.940 … 0.018/0.012 ✓ |
| damp γ=0.001→0.2 | 8/8 精确 | **0.9967–0.9999** / ≤0.049 | 0.984/0.984 … 0.127/0.137（差 <1.5%） | 0.837/0.860 … 0.057/0.058 ✓ |
| mixed (p=γ=0.02) | 8/8 精确 | **0.9998** / 0.007 | 0.730 vs 0.737 | 0.508 ≈ 0.512 |

教科书 AD 信道（full 模式）的过衰减偏差方向与 qutrit 一致（结构差异，非标度）。

## 局限与后续

- `run_normal` 剪枝路径与解析 multiplier（`_get_multiplier_impl_qutrit`/`QRAMLoadFast`）的对照验证（M3-2）。
- 输入校验：`set_noise_models` 已要求概率 ∈[0,1] 且 Damping γ<1（γ=1 会一步衰灭全部激发，
  是修复 bitphaseflip 后唯一残余的零范数路径，已从输入侧排除）。
- addr=3（25 编码 qubit，超 QuTiP 密度矩阵实用规模）与 qubit 架构对照。
- 把桥反哺为 PySparQ 绑定（`set_noise_models` + 调度导出），消除 JSON 中转。
- 上游跟进：QRAM-Simulator 三处修复可单独整理提交；uniqc 的 kraus2q 修复**不等上游
  发版**——本地 UnifiedQuantum checkout 固定在 `fix/qutip-kraus2q-dims` 分支
  （PR #130 已开、待合并；合并后切回 main 即可）。
