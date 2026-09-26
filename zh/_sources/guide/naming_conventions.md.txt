# SparQ 算子命名规范

本规范定义 QRAM-Simulator 中量子算子及相关公共符号的命名规则,是新增算子命名的**权威依据**。
规范日期:2026-09-17。配套审计对照表见文末;弃用别名机制见第 10 节。

历史背景:本规范首次成文时,存量约 60 个算子类的命名携带大量事实规则但从未成文,
并存在若干"名不副实"的标签与格式异常。首批按本规范执行的改名共 19 项(见审计表),
旧名全部保留为弃用别名。

## 1. 适用范围与符号分层

本规范以**幺正算子**为主体,其余公共符号分层适用不同强度的规则:

| 层级 | 定义 | 规则强度 |
|---|---|---|
| 幂等算子层 | `BaseOperator` / `SelfAdjointOperator` 的子类 | **强制**(第 2-9 节全部规则) |
| 状态管理层 | 寄存器管理 callable:`AddRegister`、`RemoveRegister`、`SplitRegister`、`CombineRegister`、`MoveBackRegister`、`Push`、`Pop`、`StateLoad` 等 | 仅适用大小写规则(第 2 节) |
| 测量与读出层 | 非幺正/只读:`MeasureZ`、`Reset`、`Probability`、`PartialTrace*` | 仅适用大小写规则 |
| 调试与校验层 | `Check*`、`View*`、`StatePrint`、`ModuleInheritance_Test*`、`TestRemovable` | 仅适用大小写规则;动词前缀取自封闭集 {Check, View, Print, Test, Init} |
| 比较 functor 层 | 干涉分桶用 `State{Hash,Equal,Less}Except*` | 仅适用大小写规则;`Except` = "比较时排除" |
| 自由函数层 | `split_systems`、`combine_systems`、`merge_system`、`remove_system`、`stateprep_unitary_build_schmidt` 等非算子工具 | **蛇形命名豁免**(见第 2 节) |

分层规则只约束各层内部一致性,不强制跨层同形。

## 2. 大小写规则

- 算子与类名一律 **PascalCase**;禁止小写开头的类名(历史违例 `inverseQFT` 已在首批改名中消除,现为 `InverseQFT`)。
- 自由函数(非算子工具)保持 **snake_case**,这是成文豁免:它们不是算子,蛇形与算子帕形形成天然的类别边界。C++ 通行惯例亦如此。
- 枚举值(`UnsignedInteger`、`Detail`、`Prob` 等)与宏(`UPPER_CASE`)遵循各自既有惯例。

## 3. 算子命名文法

```text
OperatorName ::= Family [ "_" Slot { "_" Slot } ] [ Variant { Variant } ]

Family       ::= PascalCase 动词短语或既有物理名词
               (Add、Swap、Compare、Hadamard、QFT、CondRot、Rot、…)

Slot         ::= TypeTag | Modifier TypeTag
TypeTag      ::= "UInt" | "Int" | "Bool" | "Rational" | "General"
Modifier      ::= "Const" | "Any" | "Fixed"

Variant      ::= "_InPlace" | "_Full" | "_Fast" | "_Partial"
```

约束:

- **TypeTag 词表封闭**,且必须与 `StateStorageType` 一一对应
  (`Common/include/typedefs.h` 的 `get_type_str`:General/UInt/Int/Bool/Rat)。
  `UInt` = 无符号整数,`Int` = 有符号整数(二补码),`Bool` = 单比特,`Rational` = 定点有理数,`General` = 不指定数值意义的原始位串。
- **Modifier 词表封闭**:`Const`(经典编译期常量,如 `ConstUInt`)、
  `Any`(类型多态,如 `AnyInt` = 有/无符号皆可)、
  `Fixed`(定点编码变体,如 `CondRot_Fixed_Bool`,非 `StateStorageType` 成员,仅作编码修饰)。
- **Variant 词表封闭,恒居名尾,PascalCase**。新增变体必须先修订本规范。
- 类型词之间一律用下划线分隔;禁止 `PartialQubit` 这类无下划线的帕形后缀
  (历史违例已改为 `Hadamard_Partial`)。

**槽位语义承载规则**(2026-09 增补,详见 `docs/operators.md`《宽度与截断约定》):
槽位类型不仅约束寄存器声明,还**决定读扩展语义**——`_UInt_` 零扩展、`_SInt_`
符号扩展、`AnyInt` 按寄存器声明类型扩展。语义由算子名承载,release 构建同样成立。

**谓词(flag)算子族**:输出为 1-bit Boolean flag 的谓词算子沿用
`Compare_UInt_UInt(l, r, less, equal)` 先例——Family 用谓词动词(`Less`、
`Equal`、`Carry`、`IsZero`、`Negative`)或组合动词链(`MulOverflow` =
"乘法溢出"),flag 输出按第 4 节隐式不列入槽位;仅提供宽度的 `out` 参数
同样不列。谓词在全精度域上求值。

## 4. 槽位规则

- Slot 按**构造参数顺序**列出**输入寄存器**的类型。
- **XOR 输出寄存器隐式不列**:`Add_UInt_UInt(lhs, rhs, res)` 三参只列两槽,
  输出寄存器由 XOR 语义隐含。同理,`Compare_UInt_UInt` 的 less/equal 输出 flag 不列。
- **消歧原则**:仅当同族兄弟在类型或粒度上存在差异时,标签才是必需的。
  - `Swap_Bool_Bool`(比特粒度)与 `Swap_General_General`(寄存器粒度)靠标签消歧,合法。
  - `Assign`、`FlipBools`、`CustomArithmetic` 无兄弟歧义,免标签,合法。
- **标签不得说谎**:名字里的 TypeTag 必须与构造函数类型检查、实际数值解释一致。
  历史违例已在首批改名中修正(见审计表 #13-16)。

## 5. `_Bool` 的语义

`_Bool` 在不同上下文有受控的两种含义:

1. **门族与单比特操作中** = "单比特粒度目标":`X_Bool(reg, digit)`、`Phase_Bool`、
   `Rot_Bool`、`Hadamard_Bool`、`Swap_Bool_Bool`。
2. **输出 flag**:`Compare_UInt_UInt` 的布尔输出,按第 4 节隐式不列,不出现在名字中。

禁止第三种用法(历史上有过"任意 1-qubit 寄存器"与"寄存器内某一位"混用未区分的时期,
以构造函数是否接受 `digit` 参数为准)。

## 6. `_InPlace` 契约

后缀 `_InPlace` ⟺ 该算子**修改输入寄存器**且非自共轭,必须显式实现 `dag()`
(`Add_UInt_UInt_InPlace`、`ShiftLeft_InPlace`)。

无 `_InPlace` 后缀的算术算子 ⟺ **XOR 出位**(out-of-place):结果异或写入独立输出寄存器,
由此自动幺正且自共轭(`Add_UInt_UInt`、`Mult_UInt_ConstUInt`)。

此二分是 `docs/operators.md` 算子约束表的命名学表达,两者必须保持同步。

## 7. 逆操作原则

- **dagger 优先**:非自共轭算子的逆一律优先通过 `dag()` 表达,不为其单独立类。
- 合法例外一:**`Inverse<Family>` 前缀词素**——仅当 `dag()` 确实不可用或代价悬殊时,
  允许独立逆类,命名以 `Inverse` 开头(`InverseQFT`)。
- 合法例外二:**互为 dagger 的成对类**——`ShiftLeft_InPlace` / `ShiftRight_InPlace`
  互为对方的逆,这是循环移位语义下的既定模式,成文豁免。
- `QFT` 自首批改名起提供 `dag()`(实现委托给逆变换)。

## 8. 门族规范

- 固定单比特门:**`<轴>_Bool`**,无 `gate` 中缀——`X_Bool`、`Y_Bool`、`Z_Bool`、
  `S_Bool`、`T_Bool`、`RX_Bool`、`RY_Bool`、`RZ_Bool`、`SX_Bool`、`U2_Bool`、`U3_Bool`。
  (历史形式 `<轴>gate_Bool` 已全部弃用,见别名表。)
- 参数化载体基类 `Phase_Bool`(相位 e^{iλ})与 `Rot_Bool`(任意 2×2 酉阵)不是固定门,
  天然无 `gate` 中缀。继承关系:Z/S/T ← `Phase_Bool`;RX/RY/SX/U2/U3 ← `Rot_Bool`。
- 门族构造函数统一支持 `(reg, digit, [params])` 与 `(reg, [params])`(digit 缺省 0)双形态。

## 9. 组合动词管道(RPN 读法)

多元复合操作的 Family 从右向左按数学应用顺序读:

- `Add_Mult_UInt_ConstUInt_InPlace` = `res += lhs * const`。
- `Div_Sqrt_Arccos_UInt_UInt` = `res ^= arccos(√(lhs/rhs)) / 2π`。
- `Sqrt_Div_Arccos_Int_UInt` = `res ^= arccos(lhs/√rhs) / 2π`。

动词链的每个词素都是独立数学算子;后接的 Slot 描述链的**原始输入**。

## 10. 弃用与别名机制

- **Python 侧**:`pysparq/__init__.py` 以 PEP 562 模块 `__getattr__` 提供旧名,
  访问时发出 `DeprecationWarning` 并解析到新名。别名表与改名总表一一对应。
- **C++ 侧**:改名所在的头文件在同名命名空间内提供
  `[[deprecated("use NewName")]] using OldName = NewName;`。
- **移除政策**:别名保留至下一个大版本。
- 外部消费者视角:SparQSim 仓库 `PySparQ/consumer_runtime_inventory.json` 记录的
  外部依赖旧名(如 `Xgate_Bool`)在别名期内继续可用;下游应在此期间迁移到新名。

## 11. 审计对照表

处置含义:**改名**(首批已执行,旧名为弃用别名)/ **合规**(本就符合规范)/
**豁免**(分层或消歧原则下免标签)/ **遗留**(记录在案的偏差,暂不处理)。

### 11.1 首批改名(19 项)

| # | 旧名 | 新名 | 依据 |
|---|---|---|---|
| 1 | `Xgate_Bool` | `X_Bool` | 门族去 `gate` 中缀(第 8 节);外部依赖名,别名承载 |
| 2 | `Ygate_Bool` | `Y_Bool` | 同上 |
| 3 | `Zgate_Bool` | `Z_Bool` | 同上 |
| 4 | `Sgate_Bool` | `S_Bool` | 同上 |
| 5 | `Tgate_Bool` | `T_Bool` | 同上 |
| 6 | `RXgate_Bool` | `RX_Bool` | 同上 |
| 7 | `RYgate_Bool` | `RY_Bool` | 同上 |
| 8 | `RZgate_Bool` | `RZ_Bool` | 同上 |
| 9 | `SXgate_Bool` | `SX_Bool` | 同上 |
| 10 | `U2gate_Bool` | `U2_Bool` | 同上 |
| 11 | `U3gate_Bool` | `U3_Bool` | 同上 |
| 12 | `inverseQFT` | `InverseQFT` | 唯一小写开头类名(第 2 节);`QFT` 同时补 `dag()`(第 7 节) |
| 13 | `GlobalPhase_Int` | `GlobalPhase` | 标签说谎:参数为复常数,无整数寄存器;全局类免标签(第 4 节) |
| 14 | `AddAssign_AnyInt_AnyInt_InPlace` | `Add_AnyInt_AnyInt_InPlace` | 同一概念两套语法,统一进 `Add_` 族(第 3、9 节) |
| 15 | `Div_Sqrt_Arccos_Int_Int` | `Div_Sqrt_Arccos_UInt_UInt` | 标签说谎:构造函数检查 UInt,UInt |
| 16 | `Sqrt_Div_Arccos_Int_Int` | `Sqrt_Div_Arccos_Int_UInt` | 标签说谎:构造函数检查 SInt,UInt |
| 17 | `Hadamard_PartialQubit` | `Hadamard_Partial` | 变体后缀规范(第 3 节) |
| 18 | `CondRot_General_Bool_fast` | `CondRot_General_Bool_Fast` | 变体后缀帕形(第 3 节);仅 C++ 未绑定,别名 `CondRot_General_Bool` 不变 |
| 19 | `QuantumBinarySearchFast`(ns CKS) | `QuantumBinarySearch_Fast` | 变体后缀规范;`QuantumBinarySearch` 本名不动 |

核实后**不改**:`GetRotateAngle_Int_Int` 实现以 `get_complement` 做有符号解释,
`_Int_Int` 诚实,合规保留。

### 11.2 幂等算子层——合规/豁免清单

- 算术(quantum_arithmetic.h):`FlipBools`(豁免)、`Swap_Bool_Bool`、
  `ShiftLeft_InPlace`、`ShiftRight_InPlace`、`Mult_UInt_ConstUInt`、
  `Add_Mult_UInt_ConstUInt_InPlace`、`Mod_Mult_UInt_ConstUInt_InPlace`、
  `Add_UInt_UInt`、`Add_UInt_UInt_InPlace`、`Add_UInt_ConstUInt`、
  `Add_ConstUInt_InPlace`、`GetRotateAngle_Int_Int`、`Assign`(豁免)、
  `Compare_UInt_UInt`、`Less_UInt_UInt`、`Swap_General_General`、
  `GetMid_UInt_UInt`、`CustomArithmetic`(豁免)——合规。
- 门族(basic_gates.h):`Phase_Bool`、`Rot_Bool` + 改名后的 11 个固定门——合规。
- Hadamard(hadamard.h):`Hadamard_Int`、`Hadamard_Int_Full`、`Hadamard_Bool`——合规。
- QFT(qft.h):`QFT`、`QFT_Full`——合规。
- 条件旋转(condrot.h):`CondRot_Rational_Bool`、`CondRot_Fixed_Bool`——合规。
- 相位与反射(parallel_phase_operations.h):`ZeroConditionalPhaseFlip`、
  `RangeConditionalPhaseFlip`、`Reflection_Bool`——合规。
- 旋转与态制备(rot.h):`Rot_GeneralUnitary`、`Rot_GeneralStatePrep`——合规
  (此处 `General` = "任意维度/任意目标"修饰,属 Family 词素而非 TypeTag,注意区分)。
- 系统算子(system_operations.h):`ClearZero`——豁免(全局类)。
- 黑魔法(dark_magic.h):`Normalize`、`Init_Unsafe`——豁免(全局类;
  `_Unsafe` 后缀标记非物理操作,保留)。

### 11.3 其他层级

状态管理层、测量与读出层、调试与校验层、比较 functor 层、自由函数层:
全部按第 1 节分层规则处理,首批**零改名**。

### 11.4 记录在案的遗留偏差(不处理,仅供知悉)

- **QRAM 家族整体豁免**:`QRAMCircuit_qutrit` 的 `_qutrit` 蛇形命名空间消歧后缀、
  `QRAMLoadFast` 无下划线变体,均与第 3 节文法不一致,经决策**保留现状**。
- **幽灵引用**:`SparQ/include/qram.h` 注释提及的 `QRAMLoad_Qubit` 在代码库中不存在。
- `SparQ_Algorithm` 中哈密顿模拟的类名 `T` 与门 `T_Bool` 概念碰撞(前者未绑定到 Python)。
- `Sort*` 家族(`SortExceptKey` 等)为模拟器内部干涉优化操作,其命名语义
  (Except/Key/Bit/Unconditional/ByAmplitude)以此处记录为准,不套用算术文法。
- 两处同名 `QRAMCircuit` 分别住在 `qram_qutrit` / `qram_qubit` 命名空间,靠命名空间消歧。

## 维护义务

新增算子时必须:按第 3-9 节命名;若引入新的 TypeTag/Modifier/Variant 词素,
先修订本规范;若改名,按第 10 节配齐两侧别名并更新本表与 `docs/operators.md`。
