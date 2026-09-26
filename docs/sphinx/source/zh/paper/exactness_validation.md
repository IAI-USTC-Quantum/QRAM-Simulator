# 剪枝与全量演化的精确性验证

`QRAMCircuitQubit` 提供全量演化（`FULL_VER`）和剪枝演化（`NORMAL_VER`）。
对于相同输入、存储内容和噪声历史，两种模式必须实现相同的轨迹。
剪枝通过参考组重建可预测的分支组，不为这些分支的演化引入截断或概率近似。

本文说明验证流程及其可复现程序 `Experiment_QRAM_QubitPaperExactTest`。

## 等价性要求

先用相同的配置种子构造两个电路，再分别在运行前重置为相同的轨迹种子。
以下量需要单独比较：

| 比较对象 | 要求 |
|---|---|
| 采样操作 | 噪声操作历史完全相同 |
| 随机数生成器 | 演化结束及输出采样结束时，生成器状态分别相同 |
| 阻尼跳跃 | 实际触发次数相同 |
| 测量前状态 | 先物化预测组，再比较带输入权重的全部复振幅 |
| 采样树态 | 残余树配置相同 |
| 测量后状态 | 比较归一化后、带输入权重的全部复振幅 |
| 地址概率 | 计入每个输入分支的概率权重，总概率为一 |
| 保真度 | 对相同理想输出、使用相同定义进行比较 |

分量使用 `(address, input bus, output bus, residual tree)` 作为键。
保留输入分支标签相当于逐列检查所表示的轨迹映射，避免不同输入列的误差
在单一保真度指标中抵消。分量与地址概率的断言容差为 $10^{-12}$，
保真度容差为 $10^{-9}$。这些阈值用于容纳浮点累加顺序造成的舍入差异，
不是剪枝算法的近似参数。

检查测量前状态时，必须**先物化预测组**。尚未演化的输入分量不能当作输出态比较。
检查测量后状态时，两种模式均须完成树态采样和归一化。

## 共享参考组的不变量

阻尼跳跃会投影参考组中的分量。若仍有分量存活，它们的振幅继续作为重建依据。
因此，发生一次跳跃本身不足以判定所有预测组都应清空。

若参考组没有任何存活分量，由同一参考组表示的全部预测组必须贡献零概率和零保真度。
`materialize_good_branches()` 会将这些组标记为 `annihilated`，并清除当前轨迹分量。
输入概率 `branch_probs` 保持不变，以便下一次查询重用；`reset()` 恢复轨迹初态并清除标记。
演化中预测组的占据概率估计已经包含参考组的存活范数。

每个 `Damp_Full` 操作在两种模式中都恰好消耗一次随机抽样。
qubit 的物化规则与 qutrit 的参考别名表示是不同的数据表示，应分别验证。

## 测试覆盖

| 入口 | 覆盖内容 |
|---|---|
| 默认运行 | 155 例固定种子、压力与无噪声测试；包含全地址零总线输入及采样均匀分支输入 |
| 固定历史用例 | 参考组死亡、参考组存活、投影后重用同一个电路对象 |
| `--channels` | 120 例：Damping、Depolarizing、BitFlip、PhaseFlip、BitPhaseFlip；$n=3,5$，$k=1,2,4$；均匀及非均匀输入权重 |
| `--perf-shard i count` | 1000 对性能网格：$k=3$，$n=4,6,8,10,12$，$\varepsilon=\gamma\in\{0,10^{-5},10^{-4},10^{-3}\}$，每点 50 个种子 |
| `--case n eps seed [uniform]` | 重放一条混合噪声轨迹；可选采样均匀分支输入 |

固定历史用例不依赖平台特定的噪声分布实现，直接覆盖参考组投影后的两种结果。
默认的 155 例测试还要求实际触发的阻尼跳跃总数不少于 50。

C++ 标准不保证不同标准库产生逐位相同的分布样本，因此 full/pruned 对照应在
相同工具链内进行。固定随机种子的特定 MSVC 跳跃结果只在 MSVC 上强制断言；
各平台均保留数值对照，并必须通过固定历史的投影用例。

## 构建与运行

在仓库根目录运行，确保 C++17 编译器和 CMake 可用：

```bash
cmake -S . -B build-exact -DCMAKE_BUILD_TYPE=Release \
  -DCACHED_REGISTER_SIZE=8 -DQRAM_BUILD_TESTS=ON \
  -DQRAM_BUILD_EXPERIMENTS=ON
cmake --build build-exact --parallel 8
OMP_NUM_THREADS=1 ctest --test-dir build-exact --output-on-failure
./build-exact/bin/CorrectnessTest
```

CTest 运行 `QubitPaperExactness`、`QubitPaperExactnessChannels` 及已有的
`verify_noisy_simulation` 电路级检查。任何断言失败均返回非零退出码。
默认精确性测试包含固定历史用例和跳跃总数覆盖断言。

Windows 多配置生成器使用 `--config Release` 构建，并运行
`ctest --test-dir build-exact -C Release --output-on-failure`。
可执行文件位于 `build-exact/bin/`，后缀为 `.exe`。

单独运行某类检查或完整性能网格：

```bash
./build-exact/bin/Experiment_QRAM_QubitPaperExactTest --fixtures
./build-exact/bin/Experiment_QRAM_QubitPaperExactTest --channels
./build-exact/bin/Experiment_QRAM_QubitPaperExactTest \
  --case 8 0.001 9208654621621431296
./build-exact/bin/Experiment_QRAM_QubitPaperExactTest --perf-shard 0 1
```

若使用八个独立进程，分别运行 `--perf-shard i 8`，其中 `i` 取 0 至 7，
并要求八个进程全部正常退出。`--battery-shard i 8` 同样可划分 155 例测试。
分片运行不执行默认测试中的跳跃总数断言和固定历史用例，因此仍应运行默认测试。
并行时使用独立进程，因为每个进程内部共用一个模拟器 RNG。

## 论文数据与稳定的 full 基线

论文原始数据应与分量级对照分别生成：

```bash
./build-exact/bin/Experiment_QRAM_QubitPaper perf build-exact/validation
./build-exact/bin/Experiment_QRAM_QubitPaper equiv build-exact/validation
```

`perf_scan.csv` 包含 2000 行，即 1000 个 `(eps, n, traj, runseed)` 键各有
一行 full 和一行 normal。检查配对完整性后再比较保真度。
CSV 检查是分量级验证的补充；保真度相等本身不能推出状态相等。

修改重建逻辑时，应使用相同编译器和配置单独构建参考版本，将其 full 行与
新版本 full 行比较。比较字段为 `arch`、`eps`、`n`、`traj`、`runseed`、
`version`、`fid`、`states`、`branches`。墙钟时间不具有确定性，
因此 `time_run_ms` 和 `time_sample_ms` 不参与一致性断言。

性能测试应控制机器负载。多个验证进程争用资源时记录的时间不应替换论文的运行时间基准。
每份数据应保留代码版本、编译器、构建选项、输入约定及种子。

## 已验证的数值参考

2026-09-26 的 CPU Release 验证使用 GCC 14.4.0/libstdc++、CMake 4.4.3、
Ninja、`CACHED_REGISTER_SIZE=8`，每个进程设置一个 OpenMP 线程。

| 检查 | 结果 |
|---|---|
| 默认种子测试 | 155/155 对通过；68 次跳跃 |
| 单通道测试 | 120/120 对通过 |
| 性能网格 | 1000/1000 对通过；64 次跳跃 |
| 性能网格测量前最大分量差 | $8.47\times10^{-16}$ |
| 性能网格测量后最大分量差 | $6.94\times10^{-16}$ |
| 性能网格最大保真度差 | $8.99\times10^{-15}$ |
| 275 例种子/通道测试的最大物理地址概率差 | $2.78\times10^{-16}$ |
| full 与参考版本 `d91e4f0` 的物理 CSV 行 | 1000/1000 完全一致 |
| 附录等价性网格 | 12/12 对通过；最大 $|\Delta F|=2.0\times10^{-15}$ |
| 已注册 CTest 套件 | 3/3 通过 |

生成的 CSV、汇总指标和日志应存放在被忽略的构建目录，例如
`build-exact/validation/`。CTest 详细日志位于
`build-exact/Testing/Temporary/LastTest.log`。

这些检查建立了所测试噪声及输入约定下与 `FULL_VER` 的数值等价性，
并为共享状态的数学论证提供验证。独立检验物理通道与验证剪枝变换是两个不同的验证任务。
