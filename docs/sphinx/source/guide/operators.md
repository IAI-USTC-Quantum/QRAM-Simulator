# 量子算术算子约束说明

本文档详细说明 QRAM-Simulator 中预实现的量子算术算子的约束条件、unitary 性质保证以及使用规范。

## 目录

- [概述](#概述)
- [宽度与截断约定](#宽度与截断约定)
- [算子分类](#算子分类)
- [算子详细说明](#算子详细说明)
- [Unitary 性质保证机制](#unitary-性质保证机制)
- [类型安全与溢出行为](#类型安全与溢出行为)
- [使用建议](#使用建议)

## 概述

QRAM-Simulator 采用 "Register Level Programming" 范式，所有算子直接操作寄存器。每个算子都经过精心设计以保证 quantum unitary 性质。

### 关键概念

1. **Out-of-place 操作**: 结果存储在独立的输出寄存器中，通过 XOR 保证 unitary
2. **In-place 操作**: 结果直接修改输入寄存器，需要显式实现 dagger 方法保证可逆性
3. **SelfAdjoint 算子**: 满足 U† = U，应用两次等于恒等操作

## 宽度与截断约定

本节是所有算术算子位宽行为的**权威契约**(2026-09 制定)。原则:**语义由算子名承载,
不依赖构建模式**;寄存器声明类型的校验仅在 debug 构建额外执行。

1. **LSB 对齐**:所有操作数与输出按最低有效位对齐,不存在隐式移位。
2. **读扩展由名字槽位决定**:`_UInt_` 槽把操作数零扩展到计算域;`_SInt_` 槽按
   二补码符号扩展;`_Bool_` 槽按单比特;`AnyInt` 槽按寄存器**声明类型**扩展
   (UInt→零扩展,SInt→符号扩展)。实现把扩展逻辑直接写死,release 构建同样正确。
3. **计算域**:中间量按全精度求值——乘法经 64 位高低半分解(等价 128 位精度),
   加/减/比较在无符号 64 位回绕域上等价成立,除法/开方商域 ≤ 64 位。
4. **写入**:结果取 `mod 2^out_width` 后 **XOR** 进输出寄存器。输出宽度可与
   任意输入宽度不同;输入寄存器的值永不被 out-of-place 算子修改。
5. **域外全量化**:可逆性要求算子在全部基矢上是确定性函数,因此核心算子不抛
   定义域异常——`Div_UInt_UInt` 在除数为零时商取 0;`Sqrt_UInt` 只接受幅度
   (非负)输入。定义域/溢出信息一律由专用 flag 算子另行报告。
6. **flag 谓词**:flag 输出 XOR 进 1-bit Boolean 寄存器;谓词在**全精度域**上
   求值。以 `out` 为参数的 flag 算子(`Carry_UInt_UInt` 等)**只取该寄存器的
   宽度、不读其值**,因此对输出寄存器的初始内容没有前置假设。
7. **宽度边界**:1..64 全支持。所有 `pow2(w)`/`1<<w` 于 w=64 的未定义行为
   一律以 `width_mask`(64 特判)替代;算子读取宽度一律在执行期经
   `System::size_of`,不在构造期快照。
8. **别名约束**:输出(含 flag)与任一输入不得别名——always-on 检查(与
   debug/release 无关)。
9. **约定例外**:`GetMid_UInt_UInt` 与 `Swap_General_General` 维持等宽要求
   (中点溢出与原值交换的语义本质上依赖等宽),是本约定仅有的两个例外。
10. **存量兼容**:既有 Add 族(读零扩展 + 写 `mod 2^dst`)与本约定一致。
    唯一的历史行为修正是 `Add_AnyInt_AnyInt_InPlace`:AnyInt 槽自本约定起按
    寄存器声明类型扩展(此前一律按无符号位模式读),并补上 `lhs == rhs` 的
    别名拒绝与执行期宽度读取。

## 算子分类

### 1. Out-of-place 算子 (SelfAdjointOperator)

| 算子 | 操作 | 输入类型 | 输出类型 | 约束 |
|------|------|----------|----------|------|
| `Add_UInt_UInt` | res ^= lhs + rhs | UnsignedInteger | UnsignedInteger | 所有寄存器大小任意，结果截断 |
| `Add_UInt_ConstUInt` | res ^= lhs + const | UnsignedInteger | UnsignedInteger | 所有寄存器大小任意，结果截断 |
| `Mult_UInt_ConstUInt` | res ^= lhs * const | UnsignedInteger | UnsignedInteger | const 应为奇数以保证双射 |
| `Assign` | reg2 ^= reg1 | 任意 | 与 reg1 相同 | 宽度任意，reg2 按 mod 2^reg2_w 截断 |
| `Compare_UInt_UInt` | 输出比较标志 | UnsignedInteger | Boolean | 输出寄存器大小为 1 |
| `Less_UInt_UInt` | 输出小于标志 | UnsignedInteger | Boolean | 输出寄存器大小为 1 |
| `GetMid_UInt_UInt` | mid ^= (l+r)/2 | UnsignedInteger | UnsignedInteger | 三个寄存器大小必须相同 |
| `FlipBools` | reg ^= ~reg | UnsignedInteger/SignedInteger | - | 按位取反所有位 |
| `Swap_Bool_Bool` | 交换单个比特 | 任意 | 任意 | 位索引在有效范围内 |
| `Swap_General_General` | 交换整个寄存器 | 任意 | 任意 | 两寄存器大小必须相同 |
| `Div_Sqrt_Arccos_UInt_UInt` | res ^= arccos(√(l/r)) | UnsignedInteger | Rational | lhs < rhs |
| `Sqrt_Div_Arccos_Int_UInt` | res ^= arccos(l/√r) | SignedInteger, UnsignedInteger | Rational | \|lhs\| ≤ √rhs |
| `GetRotateAngle_Int_Int` | res ^= atan2(r,l)/2π | 整数类型 | Rational | 结果在 [0,1) 范围内 |
| `Sub_UInt_UInt` | res ^= lhs − rhs | UnsignedInteger | UnsignedInteger | 宽度任意（约定通用行） |
| `Neg_UInt` | res ^= 0 − reg | UnsignedInteger | UnsignedInteger | 宽度任意 |
| `Abs_SInt` | res ^= \|sext(reg)\| | SignedInteger | UnsignedInteger | 补码最小值回绕自身 |
| `Mul_UInt_UInt` | res ^= lhs · rhs | UnsignedInteger | UnsignedInteger | 宽度任意，结果 mod 2^res_w |
| `Div_UInt_UInt` | res ^= lhs // rhs | UnsignedInteger | UnsignedInteger | **除零 → 商 0**（域外全量化） |
| `Sqrt_UInt` | res ^= isqrt(reg) | UnsignedInteger | UnsignedInteger | 整数开方（floor） |
| `Select_Bool_UInt_UInt` | res ^= cond ? lhs : rhs | Boolean, UInt×2 | UnsignedInteger | cond 宽度为 1 |
| `And_UInt_UInt` / `Or_UInt_UInt` / `Xor_UInt_UInt` | 按位与/或/异或 | UnsignedInteger×2 | UnsignedInteger | 宽度任意 |
| `Less_SInt_SInt` | flag ^= sext(lhs) < sext(rhs) | SignedInteger×2 | Boolean | 有符号比较 |
| `Carry_UInt_UInt` | flag ^= lhs+rhs ≥ 2^res_w | UnsignedInteger×2 | Boolean | res 仅提供宽度，不读不写 |
| `Overflow_SInt_SInt` | flag ^= 有符号加溢出 | SignedInteger×2 | Boolean | res 仅提供宽度；内部按 res 宽度求值 |
| `MulOverflow_UInt_UInt` | flag ^= lhs·rhs ≥ 2^res_w (全精度) | UnsignedInteger×2 | Boolean | res 仅提供宽度 |
| `IsZero_UInt` | flag ^= reg == 0 | UnsignedInteger | Boolean | — |
| `Negative_SInt` | flag ^= sext(reg) < 0 | SignedInteger | Boolean | — |
| `CustomArithmetic` | res ^= func(inputs) | 任意 | 任意 | func 必须是确定性函数 |

### 2. In-place 算子 (BaseOperator)

| 算子 | 操作 | 输入类型 | Dagger 实现 | 约束 |
|------|------|----------|-------------|------|
| `Add_UInt_UInt_InPlace` | rhs += lhs | UnsignedInteger | rhs += (2^N - lhs) | 宽度任意，lhs 零扩展，rhs 按 mod 2^rhs_w 回绕 |
| `Add_Mult_UInt_ConstUInt_InPlace` | res += lhs * const (lhs不变) | UnsignedInteger | res -= lhs * const | lhs不变，仅res更新 |
| `Add_ConstUInt_InPlace` | reg += const | UnsignedInteger/SignedInteger | reg += (2^N - const) | 模 2^N 回绕 |
| `Mod_Mult_UInt_ConstUInt_InPlace` | y = y * a^(2^x) mod N | UnsignedInteger | y = y * a^(-2^x) mod N | gcd(a, N) = 1，寄存器 ≥ ⌈log₂(N)⌉ |
| `Add_AnyInt_AnyInt_InPlace` | lhs += rhs | 整数类型 | lhs -= rhs (mod 2^N) | 混合类型；rhs 按声明类型扩展（SInt 符号扩展）；lhs≠rhs |
| `ShiftLeft_InPlace` | 循环左移 | UnsignedInteger/SignedInteger | 循环右移相同位数 | 移位量 ≤ 寄存器大小 |
| `ShiftRight_InPlace` | 循环右移 | UnsignedInteger/SignedInteger | 循环左移相同位数 | 移位量 ≤ 寄存器大小 |

## 算子详细说明

### Add_UInt_UInt

**操作**: `res ^= lhs + rhs` (out-of-place)

**Unitary 保证**: 通过 XOR 实现。`res ⊕ (lhs+rhs) ⊕ (lhs+rhs) = res`

**约束条件**:
- `lhs`, `rhs`, `res` 必须是 UnsignedInteger 类型
- 所有寄存器必须是激活状态
- 加法结果按 `res` 寄存器大小截断

**示例**:
```cpp
auto lhs = System::add_register("lhs", UnsignedInteger, 4);
auto rhs = System::add_register("rhs", UnsignedInteger, 4);
auto res = System::add_register("res", UnsignedInteger, 4);
Init_Unsafe(lhs, 3);
Init_Unsafe(rhs, 5);
// res = 0 ⊕ (3 + 5) = 8
Add_UInt_UInt("lhs", "rhs", "res");
```

### Add_UInt_UInt_InPlace

**操作**: `rhs += lhs` (mod 2^N) (in-place)

**Unitary 保证**: 通过模运算实现 dagger。`rhs = (rhs + (2^N - lhs)) mod 2^N` 恢复原值。

**约束条件**:
- `lhs`, `rhs` 必须是 UnsignedInteger 类型
- 强烈建议两寄存器大小相同
- 所有寄存器必须是激活状态

**溢出行为**: 结果按 `rhs` 寄存器大小模 2^N 回绕

**示例**:
```cpp
auto lhs = System::add_register("lhs", UnsignedInteger, 4);
auto rhs = System::add_register("rhs", UnsignedInteger, 4);
Init_Unsafe(lhs, 7);
Init_Unsafe(rhs, 3);
// rhs = (3 + 7) % 16 = 10
Add_UInt_UInt_InPlace("lhs", "rhs");
// dagger: rhs = (10 + 9) % 16 = 3 (恢复原值)
op.dag(state);
```

### ShiftLeft_InPlace / ShiftRight_InPlace

**操作**: 循环移位

**Unitary 保证**: 循环移位是双射，ShiftLeft_InPlace 和 ShiftRight_InPlace 互为 dagger。`ShiftLeft_InPlace::dag()` 调用 `ShiftRight_InPlace`，反之亦然。

**约束条件**:
- 寄存器必须是 UnsignedInteger 或 SignedInteger 类型
- 移位量必须 ≤ 寄存器大小
- 移位量等于大小时等于恒等操作

**示例**:
```cpp
auto reg = System::add_register("reg", UnsignedInteger, 4);
Init_Unsafe(reg, 0b1010);  // 10
// 循环左移1位: 0b0101 = 5
ShiftLeft_InPlace("reg", 1);
// 循环右移1位回到原值
ShiftRight_InPlace("reg", 1);
// 或者使用 .dag()：
ShiftLeft_InPlace("reg", 1).dag(state);  // 撤销
```

### Mult_UInt_ConstUInt

**操作**: `res ^= lhs * mult` (out-of-place)

**Unitary 保证**: 通过 XOR 实现。注意：只有当 `mult` 与 2^N 互质时，乘法才是双射。

**约束条件**:
- `lhs`, `res` 必须是 UnsignedInteger 类型
- 为保证 unitary，`mult` 应为奇数（与 2^N 互质）
- 乘法结果按 `res` 寄存器大小截断

**警告**: 如果 `mult` 为偶数，乘法不是双射，可能导致信息丢失。例如，乘以 2 会丢失最低位。

### Assign

**操作**: `reg2 ^= reg1` (out-of-place)

**Unitary 保证**: 通过 XOR 实现，是自伴操作。

**语义说明**: 这不是经典意义上的赋值。如果 `reg2` 初始为 0，效果为复制；否则为 XOR 更新。

**约束条件**:
- 两个寄存器大小必须相同
- 类型可以不同（位模式被复制）

### Compare_UInt_UInt

**操作**: `|l>|r>|0>|0> → |l>|r>|l<r?>|l==r?>`

**Unitary 保证**: 通过 XOR 设置标志位，是自伴操作。

**约束条件**:
- `left`, `right` 必须是 UnsignedInteger
- `compare_less`, `compare_equal` 必须是 Boolean（大小为 1）

### CustomArithmetic

**操作**: `outputs ^= func(inputs)` (out-of-place)

**Unitary 保证**: 通过 XOR 实现，但用户必须确保 `func` 是确定性的。

**约束条件**:
- `func` 必须是纯函数（相同输入总是产生相同输出）
- `func` 不应该有副作用
- 用户负责确保 `func` 的正确性

## Unitary 性质保证机制

### 1. XOR 机制 (Out-of-place)

所有 out-of-place 算子使用 XOR 写入结果：

```cpp
output.value ^= compute_result(input_values);
```

因为 `x ⊕ y ⊕ y = x`，应用两次操作会恢复原值，保证 U† = U。

### 2. 模运算机制 (In-place)

In-place 算子使用模运算保证可逆性：

**正向操作**:
```cpp
reg.value = (reg.value + value) % (1ULL << N);
```

**Dagger 操作**:
```cpp
reg.value = (reg.value + ((1ULL << N) - value)) % (1ULL << N);
```

因为 `(x + y) + (2^N - y) ≡ x (mod 2^N)`，操作和 dagger 互相抵消。

### 3. 双射验证

对于 out-of-place 算子，使用 `verify_outofplace_unitarity` 模板验证：
- 对所有可能的输入，应用两次操作
- 验证结果等于原始状态

对于 in-place 算子，使用 `verify_inplace_unitarity` 模板验证：
- 对所有可能的输入，应用操作然后应用 dagger
- 验证结果等于原始状态

## 类型安全与溢出行为

### Debug 模式检查

在非 `QRAM_Release` 模式下，所有算子构造函数会检查：
- 输入/输出寄存器类型是否符合要求
- 寄存器大小是否匹配（对于需要匹配的操作）
- 操作数是否在有效范围内

### 溢出行为

1. **加法/减法**: 模 2^N 回绕
2. **乘法**: 截断到输出寄存器大小
3. **移位**: 循环移位（溢出位循环到另一边）

### 类型转换

- UnsignedInteger: 直接使用原始值
- SignedInteger: 使用补码表示
- Rational: 编码为定点小数
- Boolean: 视为 1 位 UnsignedInteger

## 使用建议

### 1. 寄存器大小选择

- 确保输出寄存器有足够位数存储结果
- 对于 in-place 操作，强烈建议使用相同大小的寄存器
- 考虑溢出行为对算法正确性的影响

### 2. Unitary 验证

- 使用提供的测试模板验证自定义算子的 unitary 性
- 对于关键操作，在算法中使用 `CheckNormalization` 验证状态归一化

### 3. 性能优化

- Debug 模式的类型检查有运行时开销，Release 模式自动移除
- Out-of-place 操作通常比 in-place 更容易优化
- 考虑使用 `CustomArithmetic` 减少寄存器分配

### 4. 常见陷阱

- **乘法非双射**: 偶数乘数会导致信息丢失
- **In-place 大小不匹配**: 较小寄存器的值会被截断
- **XOR 语义**: Out-of-place 算子的输出是 XOR 到目标寄存器，不是直接赋值
- **有符号溢出**: C++ 有符号整数溢出是未定义行为，使用时需谨慎

## 参考

- [量子计算中的可逆计算](https://en.wikipedia.org/wiki/Reversible_computing)
- [Unitary 矩阵](https://en.wikipedia.org/wiki/Unitary_matrix)
- 项目论文: arXiv:2503.13832, arXiv:2503.15118
