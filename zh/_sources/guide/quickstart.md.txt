# 快速上手

## Python：噪声 QRAM 装载保真度仿真

```python
from qram_simulator import (
    QRAMCircuitQubit, OperationType, set_seed,
)

# 1. 固定随机种子（内存生成、输入采样、输出采样全部走全局引擎）
set_seed(42)

# 2. 构造 QRAM 电路：addr_size=4（16 个地址），data_size=2（每槽 4 种取值）
qram = QRAMCircuitQubit(4, 2)
qram.set_memory_random()          # 随机数据树

# 3. 注入噪声（可选）：概率须在 [0,1]，Damping 要求 gamma < 1
qram.set_noise_models({
    OperationType.Depolarizing: 1e-3,
    OperationType.Damping: 1e-4,
})

# 4. 采样 100 条输入分支并运行（剪枝模式）
qram.set_input_uniform(100)
qram.run_normal()                 # 或 run_full()：不剪枝 ground truth

# 5. 采样输出并计算装载保真度
fidelity = qram.sample_and_get_fidelity()
print(f"装载保真度 = {fidelity:.4f}")
```

无噪声时的 sanity check：保真度应精确回到 1。

```python
set_seed(7)
qram = QRAMCircuitQubit(3, 2)
qram.set_memory_random()
qram.set_input_uniform(20)
qram.run_normal()
assert abs(qram.sample_and_get_fidelity() - 1.0) < 1e-9
```

## Python：qutrit 架构

接口与 qubit 架构同形，可直接替换：

```python
from qram_simulator import QRAMCircuitQutrit, OperationType, set_seed

set_seed(7)
qram = QRAMCircuitQutrit(3, 2)
qram.set_memory_random()
qram.set_noise_models({OperationType.Depolarizing: 1e-3})
qram.set_input_uniform(20)
qram.run_normal()
print(qram.sample_and_get_fidelity())
```

> 注意：qutrit 架构的 `run(version)` 分发入口要求先设置**非空**噪声
> 模型；无噪声场景请直接调用 `run_normal()` / `run_full()`。

## Python：嵌入外部全振幅模拟器（QRAMFullAmp）

`QRAMFullAmp` 把一次 QRAM 装载复合到全振幅态向量上——外部库提供
态向量与比特映射即可：

```python
from qram_simulator import QRAMFullAmp, OperationType, set_seed

set_seed(5)
# 地址比特 {0,1}，数据比特 {2,3}，数据树 [0,1,2,3]
manipulator = QRAMFullAmp(2, 2, [0, 1, 2, 3])
manipulator.set_noise_models({OperationType.Depolarizing: 1e-3})

state = [0j] * 16
for addr in range(4):
    state[addr] = 0.5 + 0j      # 地址均匀叠加、数据位为 0

out = manipulator.apply(
    state,
    address_qubits=[0, 1],
    data_qubits=[2, 3],
    other_qubits=[],
    version="normal",           # "normal" 剪枝 / "full" 不剪枝
)
# 装载后 |a>|00> → |a>|mem[a]>：四个基矢各 1/4 概率
```

## Python：查看调度（TimeStep）

```python
from qram_simulator import TimeStep, ARCH_QUBIT, OperationType

ts = TimeStep(2, 1)                                  # addr=2, data=1
noise_free = ts.generate({}, ARCH_QUBIT)             # 无噪调度
noisy = ts.generate({OperationType.Depolarizing: 1e-3}, ARCH_QUBIT)

print(len(noise_free), "个时间片")
print(noisy)                                          # 可读调度串

lo, hi = ts.get_bad_range_qubit(0)                    # 坏分支地址区间
print(f"地址 0 出错时坏分支区间 = [{lo}, {hi}]")
```

## C++：最小示例

```cpp
#include "qram_circuit_qubit.h"
#include <iostream>

using namespace qram_simulator;
using namespace qram_qubit;

int main() {
    random_engine::set_seed(42);

    QRAMCircuit qram(4, 2);
    qram.set_memory_random();

    qram.set_noise_models({
        { OperationType::Depolarizing, 1e-3 },
        { OperationType::Damping, 1e-4 },
    });

    qram.set_input_uniform(100);
    qram.run_normal();

    std::cout << "装载保真度 = "
              << qram.sample_and_get_fidelity() << std::endl;
    return 0;
}
```

编译（假设仓库根目录为当前目录）：

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
```

## 下一步

- [架构文档](architecture.md)：模块划分、数据流与剪枝机制
- {doc}`C++ API 参考 <../api/cpp>`：全部核心类的 Doxygen 文档
- [论文复现指南](../paper/reproduction.md)：arXiv:2503.13832 图件复现
