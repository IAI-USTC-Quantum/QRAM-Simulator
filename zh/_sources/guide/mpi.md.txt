# MPI 系综并行

`qram_simulator.mpi` 子包为噪声 QRAM 模拟负载提供轨迹级（系综级）的 MPI 并行执行。

## 并行的是什么

噪声系综基准负载是一个蒙特卡洛循环：跑一条剪枝噪声轨迹（`run_normal`）、采样、
读该次采样的条件输出分布与保真度统计，重复 `runs` 次后取平均。各轨迹统计独立，
因此系综是天然的并行单元：

- **循环 rank 分片**——rank `r` 执行轨迹 `[r::size]`（逐轨迹代价波动时也均衡）；
- **逐轨迹确定性重播**——每条轨迹前 `set_seed(seed + index)`，该轨迹的结果只依赖
  全局种子与其序号，与进程数、分片方式无关；
- **rank 0 聚合**——逐轨迹样本按轨迹序号重排（跨分片逐位一致）；直方图按 rank
  顺序对各 rank 局部累加求和，与串行全局顺序求和仅差浮点结合序（相对 ~1e-16）。

## 如实口径

这是**单机多进程 CPU** 系综并行：

- Python 绑定为 CPU-only——每个 MPI rank 是一个 CPU 进程，运行元数据里如实记录
  `gpu_per_rank = 0`（绝不臆写）；
- **不是**多 GPU 执行，也**不是**稀疏态分布式切分（C++ 核心无 MPI 路径，态从不
  跨 rank 切分——被分布的只有轨迹系综）。

## 用法

```python
import qram_simulator as qs
from qram_simulator.mpi import run_ensemble, world_comm

comm = world_comm()   # mpirun 下为 mpi4py COMM_WORLD；否则 SerialComm
res = run_ensemble(qs, depth=20, data_size=1,
                   noise={qs.OperationType.PhaseFlip: 1e-5},
                   runs=100, seed=20261105,
                   memory=memory_list,      # None -> rank 0 抽取并广播
                   comm=comm)
if res is not None:   # 仅 rank 0；其余 rank 返回 None
    hist = res["hist"]             # 归一化后的 (2^depth, 2^data_size) 直方图
    fids = res["overlap_fid"]      # 逐轨迹样本，按序号排序
    meta = res["mpi"]              # nranks / gpu_per_rank / implementation / ...
```

```bash
mpirun -np 4 python noisy_ensemble.py    # 每个 rank 各进入脚本一次
```

`mpi4py` 为运行时可选依赖。缺失时——或未用进程管理器启动时——`world_comm()`
返回 `size == 1` 的 `SerialComm`，同一代码路径串行退化执行。正是这一退化模式
使模块在无 MPI 环境下保持可导入、可在 CI 中测试。

## 可复现性约定

给定相同的 `seed`、`runs` 与 `memory`：

- 逐轨迹样本（保真度、耗时、采样失败数）对任意进程数**逐位一致**；
- `memory=None`：rank 0 先以 `set_seed(seed)` 定种子再抽取 memory 并广播，
  memory 本身亦可复现；
- `np` 进程下的聚合直方图与 `np = 1` 相比仅差浮点求和顺序。

## 扩展性与测试

强扩展按构造近线性（轨迹独立，最终 gather 前无跨 rank 通信）。实测点以
`wall_loop_max_s`（各 rank 轨迹循环墙钟的最大值，不含 memory 装载与 setup）
记录，避免固定开销稀释加速比；做纯进程扩展性读数时固定 `OMP_NUM_THREADS=1`。

测试位于 `bindings/python/test/test_mpi_ensemble.py`：

- 逻辑层（分片、聚合确定性）处处可跑，包括无 MPI 的 CI runner；
- 绑定集成层在装有已构建包的环境运行；
- `mpirun -np 4` 集成测试仅在 `mpirun` + `mpi4py` 齐备处运行，其余环境自动跳过。
