# Quantum Arithmetic Operator Constraints

This document details the constraints of the quantum arithmetic operators pre-implemented in QRAM-Simulator, their unitarity guarantees, and their usage rules.

## Contents

- [Overview](#overview)
- [Width and Truncation Conventions](#width-and-truncation-conventions)
- [Operator Classification](#operator-classification)
- [Detailed Operator Descriptions](#detailed-operator-descriptions)
- [Unitarity Guarantee Mechanisms](#unitarity-guarantee-mechanisms)
- [Type Safety and Overflow Behavior](#type-safety-and-overflow-behavior)
- [Usage Recommendations](#usage-recommendations)

## Overview

QRAM-Simulator adopts the "Register Level Programming" paradigm: every operator acts directly on registers. Each operator is carefully designed to preserve the quantum unitary property.

### Key Concepts

1. **Out-of-place operations**: the result is stored in a separate output register; unitarity is guaranteed via XOR
2. **In-place operations**: the result modifies the input register directly; an explicit dagger method is required to guarantee reversibility
3. **SelfAdjoint operators**: satisfy U† = U; applying the operator twice is the identity

## Width and Truncation Conventions

This section is the **authoritative contract** for the width behavior of every arithmetic operator (established 2026-09). Principle: **semantics are carried by the operator name, not by the build mode**; validation of register declared types runs only as an additional step in debug builds.

1. **LSB alignment**: all operands and outputs are aligned at the least significant bit; there are no implicit shifts.
2. **Read extension is decided by the name slots**: a `_UInt_` slot zero-extends the operand into the computation domain; an `_SInt_` slot sign-extends in two's complement; a `_Bool_` slot reads a single bit; an `AnyInt` slot extends according to the register's **declared type** (UInt → zero-extension, SInt → sign-extension). The extension logic is hard-coded, so release builds behave identically.
3. **Computation domain**: intermediate quantities are evaluated at full precision — multiplication goes through a 64-bit high/low-half decomposition (equivalent to 128-bit precision), addition/subtraction/comparison hold equivalently over the unsigned 64-bit wrapping domain, and the quotient domain of division/square root is ≤ 64 bits.
4. **Write**: the result is reduced `mod 2^out_width` and then **XOR**-ed into the output register. The output width may differ from any input width; the values of input registers are never modified by an out-of-place operator.
5. **Totalization outside the domain**: reversibility requires the operator to be a deterministic function on every basis state, so the core operators never raise domain exceptions — `Div_UInt_UInt` returns quotient 0 when the divisor is zero; `Sqrt_UInt` accepts only magnitude (non-negative) inputs. Domain/overflow information is invariably reported by dedicated flag operators.
6. **Flag predicates**: flag outputs are XOR-ed into 1-bit Boolean registers; predicates are evaluated over the **full-precision domain**. Flag operators that take `out` as a parameter (`Carry_UInt_UInt` etc.) **take only that register's width and never read its value**, so they make no assumption about the initial content of the output register.
7. **Width bounds**: 1..64 are all supported. Every `pow2(w)` / `1<<w` undefined behavior at w=64 is replaced by `width_mask` (with a special case for 64); operators always read widths at execution time via `System::size_of` and never snapshot them at construction time.
8. **Aliasing constraint**: the output (including flags) must not alias any input — an always-on check, independent of debug/release.
9. **Exceptions to the contract**: `GetMid_UInt_UInt` and `Swap_General_General` keep their equal-width requirement (the semantics of midpoint overflow and whole-register exchange inherently depend on equal widths); they are the only two exceptions to this contract.
10. **Legacy compatibility**: the existing Add family (zero-extended reads + writes `mod 2^dst`) already conforms to this contract. The only historical behavior correction is `Add_AnyInt_AnyInt_InPlace`: from this contract onward, the AnyInt slot extends according to the register's declared type (previously it was always read as an unsigned bit pattern), and the `lhs == rhs` aliasing rejection plus execution-time width reads were added.

## Operator Classification

### 1. Out-of-place Operators (SelfAdjointOperator)

| Operator | Operation | Input Type | Output Type | Constraints |
|------|------|----------|----------|------|
| `Add_UInt_UInt` | res ^= lhs + rhs | UnsignedInteger | UnsignedInteger | all register sizes arbitrary, result truncated |
| `Add_UInt_ConstUInt` | res ^= lhs + const | UnsignedInteger | UnsignedInteger | all register sizes arbitrary, result truncated |
| `Mult_UInt_ConstUInt` | res ^= lhs * const | UnsignedInteger | UnsignedInteger | const should be odd to keep the map bijective |
| `Assign` | reg2 ^= reg1 | any | same as reg1 | widths arbitrary, reg2 truncated to mod 2^reg2_w |
| `Compare_UInt_UInt` | outputs comparison flags | UnsignedInteger | Boolean | output register size is 1 |
| `Less_UInt_UInt` | outputs the less-than flag | UnsignedInteger | Boolean | output register size is 1 |
| `GetMid_UInt_UInt` | mid ^= (l+r)/2 | UnsignedInteger | UnsignedInteger | the three registers must have the same size |
| `FlipBools` | reg ^= ~reg | UnsignedInteger/SignedInteger | - | negates all bits |
| `Swap_Bool_Bool` | swaps a single bit | any | any | bit index within the valid range |
| `Swap_General_General` | swaps entire registers | any | any | the two registers must have the same size |
| `Div_Sqrt_Arccos_UInt_UInt` | res ^= arccos(√(l/r)) | UnsignedInteger | Rational | lhs < rhs |
| `Sqrt_Div_Arccos_Int_UInt` | res ^= arccos(l/√r) | SignedInteger, UnsignedInteger | Rational | \|lhs\| ≤ √rhs |
| `GetRotateAngle_Int_Int` | res ^= atan2(r,l)/2π | integer types | Rational | result in the range [0,1) |
| `Sub_UInt_UInt` | res ^= lhs − rhs | UnsignedInteger | UnsignedInteger | widths arbitrary (generic contract row) |
| `Neg_UInt` | res ^= 0 − reg | UnsignedInteger | UnsignedInteger | widths arbitrary |
| `Abs_SInt` | res ^= \|sext(reg)\| | SignedInteger | UnsignedInteger | two's-complement minimum wraps to itself |
| `Mul_UInt_UInt` | res ^= lhs · rhs | UnsignedInteger | UnsignedInteger | widths arbitrary, result mod 2^res_w |
| `Div_UInt_UInt` | res ^= lhs // rhs | UnsignedInteger | UnsignedInteger | **division by zero → quotient 0** (totalization outside the domain) |
| `Sqrt_UInt` | res ^= isqrt(reg) | UnsignedInteger | UnsignedInteger | integer square root (floor) |
| `Select_Bool_UInt_UInt` | res ^= cond ? lhs : rhs | Boolean, UInt×2 | UnsignedInteger | cond width is 1 |
| `And_UInt_UInt` / `Or_UInt_UInt` / `Xor_UInt_UInt` | bitwise AND/OR/XOR | UnsignedInteger×2 | UnsignedInteger | widths arbitrary |
| `Less_SInt_SInt` | flag ^= sext(lhs) < sext(rhs) | SignedInteger×2 | Boolean | signed comparison |
| `Carry_UInt_UInt` | flag ^= lhs+rhs ≥ 2^res_w | UnsignedInteger×2 | Boolean | res only supplies its width; never read or written |
| `Overflow_SInt_SInt` | flag ^= signed addition overflow | SignedInteger×2 | Boolean | res only supplies its width; evaluated internally at the res width |
| `MulOverflow_UInt_UInt` | flag ^= lhs·rhs ≥ 2^res_w (full precision) | UnsignedInteger×2 | Boolean | res only supplies its width |
| `IsZero_UInt` | flag ^= reg == 0 | UnsignedInteger | Boolean | — |
| `Negative_SInt` | flag ^= sext(reg) < 0 | SignedInteger | Boolean | — |
| `CustomArithmetic` | res ^= func(inputs) | any | any | func must be a deterministic function |

### 2. In-place Operators (BaseOperator)

| Operator | Operation | Input Type | Dagger Implementation | Constraints |
|------|------|----------|-------------|------|
| `Add_UInt_UInt_InPlace` | rhs += lhs | UnsignedInteger | rhs += (2^N - lhs) | widths arbitrary; lhs zero-extended, rhs wraps mod 2^rhs_w |
| `Add_Mult_UInt_ConstUInt_InPlace` | res += lhs * const (lhs unchanged) | UnsignedInteger | res -= lhs * const | lhs unchanged, only res is updated |
| `Add_ConstUInt_InPlace` | reg += const | UnsignedInteger/SignedInteger | reg += (2^N - const) | wraps mod 2^N |
| `Mod_Mult_UInt_ConstUInt_InPlace` | y = y * a^(2^x) mod N | UnsignedInteger | y = y * a^(-2^x) mod N | gcd(a, N) = 1, register ≥ ⌈log₂(N)⌉ |
| `Add_AnyInt_AnyInt_InPlace` | lhs += rhs | integer types | lhs -= rhs (mod 2^N) | mixed types; rhs extends per its declared type (SInt sign-extends); lhs≠rhs |
| `ShiftLeft_InPlace` | rotate left | UnsignedInteger/SignedInteger | rotate right by the same amount | shift amount ≤ register size |
| `ShiftRight_InPlace` | rotate right | UnsignedInteger/SignedInteger | rotate left by the same amount | shift amount ≤ register size |

## Detailed Operator Descriptions

### Add_UInt_UInt

**Operation**: `res ^= lhs + rhs` (out-of-place)

**Unitarity guarantee**: realized through XOR. `res ⊕ (lhs+rhs) ⊕ (lhs+rhs) = res`

**Constraints**:
- `lhs`, `rhs`, `res` must be of UnsignedInteger type
- all registers must be active
- the sum is truncated to the size of the `res` register

**Example**:
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

**Operation**: `rhs += lhs` (mod 2^N) (in-place)

**Unitarity guarantee**: the dagger is realized through modular arithmetic. `rhs = (rhs + (2^N - lhs)) mod 2^N` restores the original value.

**Constraints**:
- `lhs`, `rhs` must be of UnsignedInteger type
- using registers of the same size is strongly recommended
- all registers must be active

**Overflow behavior**: the result wraps modulo 2^N at the size of the `rhs` register

**Example**:
```cpp
auto lhs = System::add_register("lhs", UnsignedInteger, 4);
auto rhs = System::add_register("rhs", UnsignedInteger, 4);
Init_Unsafe(lhs, 7);
Init_Unsafe(rhs, 3);
// rhs = (3 + 7) % 16 = 10
Add_UInt_UInt_InPlace("lhs", "rhs");
// dagger: rhs = (10 + 9) % 16 = 3 (restores the original value)
op.dag(state);
```

### ShiftLeft_InPlace / ShiftRight_InPlace

**Operation**: circular shift

**Unitarity guarantee**: a circular shift is a bijection; ShiftLeft_InPlace and ShiftRight_InPlace are each other's dagger. `ShiftLeft_InPlace::dag()` invokes `ShiftRight_InPlace`, and vice versa.

**Constraints**:
- the register must be of UnsignedInteger or SignedInteger type
- the shift amount must be ≤ the register size
- a shift amount equal to the size is the identity operation

**Example**:
```cpp
auto reg = System::add_register("reg", UnsignedInteger, 4);
Init_Unsafe(reg, 0b1010);  // 10
// rotate left by 1: 0b0101 = 5
ShiftLeft_InPlace("reg", 1);
// rotate right by 1 returns to the original value
ShiftRight_InPlace("reg", 1);
// or use .dag():
ShiftLeft_InPlace("reg", 1).dag(state);  // undo
```

### Mult_UInt_ConstUInt

**Operation**: `res ^= lhs * mult` (out-of-place)

**Unitarity guarantee**: realized through XOR. Note: the multiplication is bijective only when `mult` is coprime to 2^N.

**Constraints**:
- `lhs`, `res` must be of UnsignedInteger type
- for unitarity, `mult` should be odd (coprime to 2^N)
- the product is truncated to the size of the `res` register

**Warning**: if `mult` is even, the multiplication is not bijective and can lose information — multiplying by 2, for example, loses the lowest bit.

### Assign

**Operation**: `reg2 ^= reg1` (out-of-place)

**Unitarity guarantee**: realized through XOR; it is a self-adjoint operation.

**Semantics note**: this is not an assignment in the classical sense. If `reg2` starts at 0, the effect is a copy; otherwise it is an XOR update.

**Constraints**:
- the two registers must have the same size
- the types may differ (the bit pattern is copied)

### Compare_UInt_UInt

**Operation**: `|l>|r>|0>|0> → |l>|r>|l<r?>|l==r?>`

**Unitarity guarantee**: the flags are set through XOR; it is a self-adjoint operation.

**Constraints**:
- `left`, `right` must be UnsignedInteger
- `compare_less`, `compare_equal` must be Boolean (size 1)

### CustomArithmetic

**Operation**: `outputs ^= func(inputs)` (out-of-place)

**Unitarity guarantee**: realized through XOR, but the user must ensure that `func` is deterministic.

**Constraints**:
- `func` must be a pure function (the same inputs always produce the same outputs)
- `func` must not have side effects
- the user is responsible for the correctness of `func`

## Unitarity Guarantee Mechanisms

### 1. The XOR Mechanism (Out-of-place)

All out-of-place operators write their results via XOR:

```cpp
output.value ^= compute_result(input_values);
```

Because `x ⊕ y ⊕ y = x`, applying the operation twice restores the original value, guaranteeing U† = U.

### 2. The Modular-Arithmetic Mechanism (In-place)

In-place operators guarantee reversibility through modular arithmetic:

**Forward operation**:
```cpp
reg.value = (reg.value + value) % (1ULL << N);
```

**Dagger operation**:
```cpp
reg.value = (reg.value + ((1ULL << N) - value)) % (1ULL << N);
```

Because `(x + y) + (2^N - y) ≡ x (mod 2^N)`, the operation and its dagger cancel out.

### 3. Bijection Verification

For out-of-place operators, the `verify_outofplace_unitarity` template verifies:
- for all possible inputs, apply the operation twice
- check that the result equals the original state

For in-place operators, the `verify_inplace_unitarity` template verifies:
- for all possible inputs, apply the operation and then the dagger
- check that the result equals the original state

## Type Safety and Overflow Behavior

### Debug-Mode Checks

In any mode other than `QRAM_Release`, every operator constructor checks:
- whether the input/output register types meet the requirements
- whether the register sizes match (for operations that require matching)
- whether the operands are within the valid range

### Overflow Behavior

1. **Addition/subtraction**: wraps mod 2^N
2. **Multiplication**: truncated to the output register size
3. **Shifts**: circular shifts (overflow bits wrap around to the other side)

### Type Conversions

- UnsignedInteger: the raw value is used directly
- SignedInteger: two's-complement representation
- Rational: encoded as a fixed-point fraction
- Boolean: treated as a 1-bit UnsignedInteger

## Usage Recommendations

### 1. Choosing Register Sizes

- make sure the output register has enough bits to hold the result
- for in-place operations, registers of the same size are strongly recommended
- consider how overflow behavior affects the correctness of your algorithm

### 2. Unitarity Verification

- use the provided test templates to verify the unitarity of custom operators
- for critical operations, verify state normalization with `CheckNormalization` inside the algorithm

### 3. Performance

- debug-mode type checks carry runtime overhead; Release mode removes them automatically
- out-of-place operations are usually easier to optimize than in-place ones
- consider `CustomArithmetic` to reduce register allocation

### 4. Common Pitfalls

- **Non-bijective multiplication**: an even multiplier loses information
- **In-place size mismatch**: values in the smaller register are truncated
- **XOR semantics**: the output of an out-of-place operator is XOR-ed into the target register; it is not a plain assignment
- **Signed overflow**: signed integer overflow is undefined behavior in C++; use it with care

## References

- [Reversible computing in quantum computing](https://en.wikipedia.org/wiki/Reversible_computing)
- [Unitary matrix](https://en.wikipedia.org/wiki/Unitary_matrix)
- Project papers: arXiv:2503.13832, arXiv:2503.15118
