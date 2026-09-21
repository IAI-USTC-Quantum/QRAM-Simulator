# verify_noisy_simulation — QRAM-Simulator ↔ 电路级噪声模拟 对拍（CI 用例）

`Experiments/verify_noisy_simulation.cpp`（ctest 用例 `verify_noisy_simulation`，随
CI 全量回归自动运行，约 3 秒）：把本工作区建立的「符号稀疏树轨迹引擎 ↔ 电路级噪声
模拟」对拍固化进仓库，用一组确定性实验持续保证两种架构噪声模拟的正确性。

## 结构

- **内置独立模拟器**（与符号树引擎零共享代码）：态矢量引擎 + 稀疏密度矩阵引擎；
  门/信道统一为「局域矩阵 + 控制极性集」原语（支持任意 1q/2q/3q 局域矩阵与 Kraus 族，
  无需门分解）。
- **编码**：qutrit 节点 3 qubit（W=00/L=01/R=10/死态 11 + data）；qubit 节点 2 qubit
  （addr + data；相位 kickback 版 FetchData + CopyIn/CopyOut 的真实 Hadamard）。
- **参考侧**：同进程直接运行 `qram_qutrit::QRAMCircuit` / `qram_qubit::QRAMCircuit`
  （`run_full` 全分支轨迹）；Damp_Full 的第二次抽样由复刻执行拦截（逐 RNG draw 对齐）。

## 实验阶梯（14 组，55 项断言）

| # | 实验 | 断言 |
| --- | --- | --- |
| S0 | qutrit/qubit 无噪声桥（addr=2 全 memory） | 输出分布逐位相等（<1e-9） |
| S1 | depol p∈{0.02,0.3}、damp γ=0.05 逐随机 case（各 6 case × 两架构） | 含跳变 outcome 的次归一化分布逐位相等（<1e-16 级） |
| S2 | depol/damp/mixed 信道级 faithful 镜像（300 轨迹参考） | F_cls(归一)>0.99~0.995、TVD<0.05、\|trace−survival\|<0.01~0.02、\|F_quantum−avg_overlap_fid\|<0.05 |

全部固定 seed（确定性）；失败返回非零退出码。`VNS_TRACE`/`VNS_TRACE2`/`VNS_DEBUG`
环境变量可打开逐步激发/分布调试探针。

## 开发过程中该对拍抓到并修复的实现差异（对拍价值的直接证明）

1. 密度引擎初版的控制语义错误（控制不满足侧误用矩阵对角元而非恒等）——S2 零噪声自检
   立即暴露（trace=0）。
2. qutrit `SwapInternal{ℓ≥1}` 翻译把两个 child 都发射了受控 internal_swap——未被路由
   选中的 ground `(W,0)` 被错误激发成 `(L,0)`；对 (addr,bus) 边缘分布不可见（S0 蒙混），
   但被 Damp_Common 过度衰减（S1 阻尼范数差 ~10%）并在高噪声 depol 下进入边缘分布。
3. qutrit (a1,a0) 局域矩阵的 bit 序错位（L↔R 互换）——相位类算子在分布上不可见，
   置换类/跳变在 p=0.3 / 阻尼下暴露。
4. qubit `CopyOut` 的 Hadamard 双重发射（交换前后各一次）——`|±>` 在 node0.d 与 bus
   间复制出额外激发，边缘分布仍正确，但阻尼 S1/S2 的 trace↔survival 差 0.06；
   修复后差 0.001。

另：库侧 `qram_qubit::SystemState::run_bitphaseflip` 已按设计意图修复为 Y flip
（|0>→−|1>、|1>→|0>，与 qutrit 架构奇数位语义一致）；旧实现为秩 1 非酉算子，
是 qubit 架构高噪声下 `sample_output` 零范数崩溃的根因（详见仓库实验 README）。
两架构的 `set_noise_models` 增加概率范围校验（∈[0,1]，Damping 要求 γ<1），
从输入侧排除唯一残余的零范数路径。

这些与交互式管线（`Experiments/QRAM/ChannelCorrespondence/`，uniqc/QuTiP 后端）的
发现互相印证：**边缘分布匹配不足以保证树态正确，必须有阻尼/相干口径的验证维度。**

## 运行

```bash
cmake --build build --target verify_noisy_simulation
./build/bin/verify_noisy_simulation          # 或
ctest --test-dir build -R verify_noisy
```
