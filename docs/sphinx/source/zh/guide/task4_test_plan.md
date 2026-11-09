# 任务4 QRAM 测试方案：记录、QPU 数据访问与缓解判据

本页是 **本仓** 所承担的 QRAM 任务4子项的固定测试方案。QRAM-Simulator
只做工程封装与历史数据离线导入：**本仓不运行真机、不消耗 shots、仿真数据
绝不冒充真机数据。** 真机业务（提交、映射、随机化、恢复、位序、误差处理、
缓解执行）冻结在所属仓库（uniqc / UnifiedQuantum）；任务 3/5/6 的测试方案
归其负责人，不在本页代写。

## 被测对象与三个不可互换的功能

| 功能 | 入口 | 是什么 / 不是什么 |
|---|---|---|
| 三层 QRAM 装载 | `qram-simulator run` | 经物理树的含噪 bucket-brigade 路由（GUI 2-3 层；CLI 至深度 20）；不是读出实验 |
| QPU 寻址数据访问 | `qram-simulator addressing` | 离线标准 I/O：种子化的 2-3 层、4-8 条随机数据、均匀相干寻址、平均 `(地址:总线)` 分布；**本仓不做硬件访问** |
| 缓解仿真 | 同入口的缓解块 | 按已知数据校准的对称总线翻转混淆逐地址求逆——缓解判据的*仿真参考*，不是硬件读出缓解运行 |

## 真实 2-3 层、4-8 条随机数据

寻址入口由种子确定性导出数据表：2 层 → 4 条记录，3 层 → 8 条记录，每条
一比特，记录在 JSON 中（`memory.records`）。固定夹具（种子 20261109、256
轨迹）覆盖两档层数与全部信道——用固定命令再生：

```bash
for L in 2 3; do
  for C in none BitFlip PhaseFlip Depolarizing Damping; do
    qram-simulator addressing --layers $L --channel $C \
      --probability 0.001 --trajectories 256 --seed 20261109 \
      --out addressing-L$L-$C.json
  done
done
```

（无噪声记录使用 `--channel none --probability 0`。）

## QPU 数据访问——离线标准 I/O

`qramsim.addressing.v1` JSON 是 uniqc 侧声明的消费接口：配置、种子数据表、
资源、平均输出分布、原始/缓解正确寻址值、校准翻转率、缓解公式与显式范围
标记。导出来自 CLI（`--out`）或 GUI 面板（下载）。再导入以供核验：

```bash
qram-simulator addressing --evaluate addressing-L3-Depolarizing.json
```

`--evaluate` 执行**离线导入**：校验本软件或声明的 uniqc 消费方所产记录的
模式与必备数值字段，从记录值重算缓解寻址判据，并输出一致性标记
（`qramsim.addressing-eval.v1`）。它绝不重跑轨迹、绝不触碰硬件；对仿真
记录的评估不是硬件测试。

## 错误缓解后保真度 ≥ 75%——既有记录

判据采用任务书直接子项语义——`mitigated_correct_addressing >= 0.75`，值与
算符记录在 JSON 中（`criterion.threshold`、`criterion.operator`、
`criterion.value`、`criterion.pass`）。既有的 2 层/3 层记录（种子
20261109、p=0.001、256 轨迹）全部通过，例如：

- L3 Depolarizing：原始 0.9941 → 缓解后 **0.9985**（通过）；
- L2 BitFlip / PhaseFlip / Depolarizing / Damping 与 L3 各信道：同一工件
  集内均有记录；用上述固定命令再生并按语义比对（同种子 ⇒ 同后端上同一
  数据表与样本）。

缓解模型为按已知数据校准的对称单比特总线翻转混淆矩阵，逐地址求逆并截断到
[0, 1]：`true = ((1-e)*obs - e*obs_flipped) / (1-2e)`，`e` 为校准翻转率。
它只缓解符合该模型的路由诱发翻转——不是对任意误差模型的证书，缓解值必须
连同记录的模型与翻转率一起解读。

## 固定操作说明（GUI）

1. `qram-simulator gui` → **寻址缓解仿真（任务2.2参考）** 面板；
2. 设置层数（2/3）、信道、概率（≤ 0.1）、轨迹数（≤ 256）、种子并运行；
   显示判据结论、翻转率、分布与数据表；
3. 下载 `qram-addressing.json`（即 `qramsim.addressing.v1` 记录）；
4. 用 `qram-simulator addressing --evaluate <文件>` 离线再导入。

GUI 将该缓解仿真与含噪 QRAM 实验面板、理想宽字查询面板相互区分；三者不可
互换，界面标签保持这一区分。
