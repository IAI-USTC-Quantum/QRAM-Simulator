# 宽字稀疏查询

理想稀疏 XOR 查询接口 `qram_simulator.SparseWordMemory` 面向**任意位宽**的
逻辑地址/数据字（65、129、256 位及更高）。它与 `QRAMCircuitQubit`/
`QRAMCircuitQutrit` 的含噪 bucket-brigade 路由刻意分离：原生含噪电路把
地址+总线打包进一个 64 位分支索引，更宽的组合会**显式抛错**；稀疏字路径则以
精确任意精度整数承载每个值，绝不截断。

## 接口

```python
from qram_simulator import SparseWordMemory

memory = SparseWordMemory(address_width=129, data_width=193,
                          cells={(1 << 128) + 5: (1 << 192) + 7})
memory.xor((1 << 128) + 5, data=9)   # 经典 XOR 读取，自逆
```

- `cells` 把地址映射到非零字；映射为零或缺失的地址读出精确默认零；
- `.xor(address, data=0)` 为理想经典语义：`data XOR word`，逐位精确、自逆；
- `.apply_xor(state, address_bits, data_bits, backend, conditions=())`
  执行**相干查询**：每个置位数据比特一个受控原生 X，控制条件为合并后的
  地址/条件模式。

## 原生位坐标与算子协议

宽于一个机器字的逻辑寄存器跨多个*字寄存器*。位坐标为
`(字寄存器名, 位序号)` 对，`位序号` 限于一个 64 位机器字内；
`address_bits[i]` / `data_bits[i]` 命名逻辑第 `i` 位（小端）。后端协议
（与 SparQSim 负责人冻结；生产执行是 SparQSim 原生后端，其 CMake 从
submodule 把本仓 `word_memory.py` 安装为 `pysparq/_qram_words.py`——唯一源码）：

- `backend.X_Bool(字寄存器名, 位序号)` 返回调用时施加原生 X 的算子：
  `operator(state)`；
- `operator.conditioned_by_bit(坐标列表)` 为列出的比特加上读 1 控制——
  实现可原地修改、返回 `self` 或返回新算子，三者均受支持。

`conditions` 是 `(小端位列表, 期望值)` 对：仅当基矢的对应比特取期望值时
才查询该存储单元。与地址位重合的零控制会**约束**查询（只作用于一致的
单元），绝不改址；地址与条件冲突时该单元被完全关闭。

随包 `qram_simulator.word_backend.ReferenceWordBackend` 是基于命名字寄存器
的精确态矢量真实执行器（任意位宽基矢索引上的振幅字典），供 CLI/GUI 演示与
测试链使用。共享 fixture
`bindings/python/test/fixtures/word_query_fixture.json`
（`qram.word-query-fixture.v1`，宽整数用十进制字符串）让 QRAM-Simulator 与
SparQSim 用相同源数据互验两个原生后端。

## 容量概念

| 概念 | 含义 | 代价 |
|---|---|---|
| 宽逻辑字 | `address_width`/`data_width` 位，精确 | Python `int`，绝不截断 |
| 稀疏记录 | 一条非零 `cells` 项 | 唯一实际存储对象 |
| 名义地址空间 | `2**address_width` 个标签（`memory.nominal_address_space`） | 纯上界——不分配任何实体 |
| 可分配实体容量 | 调用方后端提供的机器字/量子比特 | 归后端所有 |

名义地址空间永不分配实体、量子比特或树节点；理想查询路径无噪声，也不构成
任何噪声路由声明。

## CLI 与 GUI

```bash
qram-simulator words --address-width 129 --data-width 193 --records 8 --inputs 4
```

以种子生成稀疏宽字内存，运行经典 XOR 与真实执行的查询链（叠加查询地址、
交替相位），自检查询链逐基矢符合经典语义、二次查询恢复初态（自逆），并写出
`qramsim.words.v1` JSON（宽整数一律十进制字符串）。GUI 的宽字面板提供同一
流程；参数变更即作废旧结果。

## 相关

含噪原生路径的 64 位边界由测试钉住
（`test_word_memory.py::test_narrow_native_boundary_is_explicit_not_silent`）：
`addr_size + data_size > 64` 在输入采样处抛错，超范围 Python 整数在绑定层
抛错而非截断。任务 2.2 的含噪寻址/缓解仿真参考是 `addressing` CLI
（`qramsim.addressing.v1`）。
