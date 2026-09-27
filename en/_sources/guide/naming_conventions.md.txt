# SparQ Operator Naming Conventions

This document defines the naming rules for quantum operators and related public symbols in QRAM-Simulator; it is the **authoritative reference** for naming new operators.
Specification date: 2026-09-17. The companion audit cross-reference table is [at the end of this document](#naming-audit); the deprecation and alias mechanism is described in [Section 10](#naming-alias).

Historical background: when this specification was first written down, the names of roughly 60 existing operator classes carried a large body of de facto rules that had never been documented, along with several misleading labels and format anomalies. The first batch of renames executed under this specification covered 19 items (see the [audit table](#naming-audit)); all old names were retained as deprecated aliases.

(naming-scope)=

## 1. Scope and Symbol Layers

This specification is primarily about **unitary operators**; the remaining public symbols fall into layers governed by rules of different strength:

| Layer | Definition | Rule Strength |
|---|---|---|
| Idempotent-operator layer | subclasses of [`BaseOperator`](https://github.com/IAI-USTC-Quantum/SparQSim/blob/main/SparQ/include/basic_components.h#L748) / [`SelfAdjointOperator`](https://github.com/IAI-USTC-Quantum/SparQSim/blob/main/SparQ/include/basic_components.h#L802) | **mandatory** ([all rules in Sections 2–9](#naming-casing)) |
| State-management layer | register-management callables: [`AddRegister`](https://github.com/IAI-USTC-Quantum/SparQSim/blob/main/SparQ/include/system_operations.h#L351), [`RemoveRegister`](https://github.com/IAI-USTC-Quantum/SparQSim/blob/main/SparQ/include/system_operations.h#L443), [`SplitRegister`](https://github.com/IAI-USTC-Quantum/SparQSim/blob/main/SparQ/include/system_operations.h#L202), [`CombineRegister`](https://github.com/IAI-USTC-Quantum/SparQSim/blob/main/SparQ/include/system_operations.h#L258), [`MoveBackRegister`](https://github.com/IAI-USTC-Quantum/SparQSim/blob/main/SparQ/include/system_operations.h#L309), [`Push`](https://github.com/IAI-USTC-Quantum/SparQSim/blob/main/SparQ/include/system_operations.h#L484), [`Pop`](https://github.com/IAI-USTC-Quantum/SparQSim/blob/main/SparQ/include/system_operations.h#L531), [`StateLoad`](https://github.com/IAI-USTC-Quantum/SparQSim/blob/main/SparQ/include/system_operations.h#L612), etc. | capitalization rules only ([Section 2](#naming-casing)) |
| Measurement and readout layer | non-unitary / read-only: [`MeasureZ`](https://github.com/IAI-USTC-Quantum/SparQSim/blob/main/SparQ/include/measurement.h#L75), [`Reset`](https://github.com/IAI-USTC-Quantum/SparQSim/blob/main/SparQ/include/measurement.h#L130), [`Probability`](https://github.com/IAI-USTC-Quantum/SparQSim/blob/main/SparQ/include/measurement.h#L197), [`PartialTrace*`](https://github.com/IAI-USTC-Quantum/SparQSim/blob/main/SparQ/include/partial_trace.h#L24) | capitalization rules only |
| Debug and verification layer | [`Check*`](https://github.com/IAI-USTC-Quantum/SparQSim/blob/main/SparQ/include/debugger.h#L78), [`View*`](https://github.com/IAI-USTC-Quantum/SparQSim/blob/main/SparQ/include/debugger.h#L182), [`StatePrint`](https://github.com/IAI-USTC-Quantum/SparQSim/blob/main/SparQ/include/debugger.h#L221), [`ModuleInheritance_Test*`](https://github.com/IAI-USTC-Quantum/SparQSim/blob/main/SparQ/include/debugger.h#L26), [`TestRemovable`](https://github.com/IAI-USTC-Quantum/SparQSim/blob/main/SparQ/include/debugger.h#L292) | capitalization rules only; verb prefixes drawn from the closed set {Check, View, Print, Test, Init} |
| Comparison functor layer | interference-bucketing [`State{Hash,Equal,Less}Except*`](https://github.com/IAI-USTC-Quantum/SparQSim/blob/main/SparQ/include/quantum_interfere_basic.h#L22) | capitalization rules only; `Except` = "excluded from the comparison" |
| Free-function layer | non-operator utilities such as [`split_systems`](https://github.com/IAI-USTC-Quantum/SparQSim/blob/main/SparQ/include/system_operations.h#L27), [`combine_systems`](https://github.com/IAI-USTC-Quantum/SparQSim/blob/main/SparQ/include/system_operations.h#L71), [`merge_system`](https://github.com/IAI-USTC-Quantum/SparQSim/blob/main/SparQ/include/basic_components.h#L598), [`remove_system`](https://github.com/IAI-USTC-Quantum/SparQSim/blob/main/SparQ/include/basic_components.h#L605), [`stateprep_unitary_build_schmidt`](https://github.com/IAI-USTC-Quantum/SparQSim/blob/main/SparQ/include/rot.h#L123) | **snake_case exemption** (see [Section 2](#naming-casing)) |

The layered rules only constrain consistency within each layer; they do not force cross-layer uniformity.

(naming-casing)=

## 2. Capitalization Rules

- Operator and class names are always **PascalCase**; class names starting with a lowercase letter are forbidden (the historical violation [`inverseQFT`](https://github.com/IAI-USTC-Quantum/SparQSim/blob/main/SparQ/include/qft.h#L80) was eliminated in the first rename batch and is now `InverseQFT`).
- Free functions (non-operator utilities) keep **snake_case** — a documented exemption: they are not operators, and the snake-case/Pascal-case contrast forms a natural category boundary. This also matches common C++ practice.
- Enum values (`UnsignedInteger`, `Detail`, `Prob`, etc.) and macros (`UPPER_CASE`) follow their own established conventions.

(naming-grammar)=

## 3. Operator Naming Grammar

```text
OperatorName ::= Family [ "_" Slot { "_" Slot } ] [ Variant { Variant } ]

Family       ::= PascalCase verb phrase or established physics noun
               (Add, Swap, Compare, Hadamard, QFT, CondRot, Rot, …)

Slot         ::= TypeTag | Modifier TypeTag
TypeTag      ::= "UInt" | "Int" | "Bool" | "Rational" | "General"
Modifier      ::= "Const" | "Any" | "Fixed"

Variant      ::= "_InPlace" | "_Full" | "_Fast" | "_Partial"
```

Constraints:

- **The TypeTag vocabulary is closed** and must correspond one-to-one with [`StateStorageType`](https://github.com/IAI-USTC-Quantum/QRAM-Simulator/blob/main/Common/include/typedefs.h#L190)
  ([`get_type_str`](https://github.com/IAI-USTC-Quantum/QRAM-Simulator/blob/main/Common/include/typedefs.h#L200) in [Common/include/typedefs.h](https://github.com/IAI-USTC-Quantum/QRAM-Simulator/blob/main/Common/include/typedefs.h): General/UInt/Int/Bool/Rat).
  `UInt` = unsigned integer, `Int` = signed integer (two's complement), `Bool` = a single bit, `Rational` = fixed-point rational, `General` = a raw bit string with no numeric meaning attached.
- **The Modifier vocabulary is closed**: `Const` (classical compile-time constant, e.g. `ConstUInt`),
  `Any` (type-polymorphic, e.g. `AnyInt` = signed or unsigned),
  `Fixed` (fixed-point encoding variant, e.g. [`CondRot_Fixed_Bool`](https://github.com/IAI-USTC-Quantum/SparQSim/blob/main/SparQ/include/condrot.h#L122); not a `StateStorageType` member, purely an encoding modifier).
- **The Variant vocabulary is closed, always sits at the end of the name, and is PascalCase**. New variants require this specification to be revised first.
- Type words are always separated by underscores; PascalCase suffixes without an underscore such as `PartialQubit` are forbidden
  (the historical violation was renamed to [`Hadamard_Partial`](https://github.com/IAI-USTC-Quantum/SparQSim/blob/main/SparQ/include/hadamard.h#L252)).

**Slot-carried semantics rule** (added 2026-09; see ["Width and Truncation Conventions" in `operators.md`](operators.md#operators-width-truncation)):
slot types do not only constrain register declarations, they also **decide the read-extension semantics** — `_UInt_` zero-extends,
`_SInt_` sign-extends, `AnyInt` extends according to the register's declared type. The semantics are carried by the operator name and hold equally in release builds.

**Predicate (flag) operator family**: predicate operators whose output is a 1-bit Boolean flag follow the
[`Compare_UInt_UInt`](operators.md#op-compare-uint-uint)`(l, r, less, equal)` precedent — the Family is a predicate verb (`Less`,
`Equal`, `Carry`, `IsZero`, `Negative`) or a compound verb chain (`MulOverflow` =
"multiplication overflow"); flag outputs are implicitly not listed as slots per [Section 4](#naming-slots), and neither
are `out` parameters that only supply a width. Predicates are evaluated over the full-precision domain.

(naming-slots)=

## 4. Slot Rules

- Slots list the types of the **input registers** in **constructor-argument order**.
- **XOR output registers are implicitly not listed**: [`Add_UInt_UInt`](operators.md#op-add-uint-uint)`(lhs, rhs, res)` takes three arguments but lists only two slots;
  the output register is implied by the XOR semantics. Likewise, the less/equal output flags of `Compare_UInt_UInt` are not listed.
- **Disambiguation principle**: labels are required only when sibling operators of the same family differ in type or granularity.
  - [`Swap_Bool_Bool`](https://github.com/IAI-USTC-Quantum/SparQSim/blob/main/SparQ/include/quantum_arithmetic.h#L116) (bit granularity) and [`Swap_General_General`](https://github.com/IAI-USTC-Quantum/SparQSim/blob/main/SparQ/include/quantum_arithmetic.h#L3054) (register granularity) are disambiguated by their labels — legal.
  - [`Assign`](operators.md#op-assign), [`FlipBools`](https://github.com/IAI-USTC-Quantum/SparQSim/blob/main/SparQ/include/quantum_arithmetic.h#L58), [`CustomArithmetic`](operators.md#op-custom-arithmetic) have no sibling ambiguity, so they are exempt from labels — legal.
- **Labels must not lie**: the TypeTag in a name must agree with the constructor's type checks and the actual numeric interpretation.
  Historical violations were fixed in the first rename batch ([audit table](#naming-audit-renames) #13–16).

(naming-bool)=

## 5. The Semantics of `_Bool`

`_Bool` has two controlled meanings depending on context:

1. **In gate families and single-bit operations** = "a single-bit-granularity target": `X_Bool(reg, digit)`, [`Phase_Bool`](https://github.com/IAI-USTC-Quantum/SparQSim/blob/main/SparQ/include/basic_gates.h#L74),
   [`Rot_Bool`](https://github.com/IAI-USTC-Quantum/SparQSim/blob/main/SparQ/include/basic_gates.h#L136), [`Hadamard_Bool`](https://github.com/IAI-USTC-Quantum/SparQSim/blob/main/SparQ/include/hadamard.h#L178), `Swap_Bool_Bool`.
2. **Output flags**: the Boolean outputs of `Compare_UInt_UInt`, implicitly not listed per [Section 4](#naming-slots) and absent from the name.

A third usage is forbidden (historically there was a period when "any 1-qubit register" and "a specific digit inside a register"
were conflated without distinction; the criterion is whether the constructor accepts a `digit` parameter).

(naming-inplace)=

## 6. The `_InPlace` Contract

The suffix `_InPlace` ⟺ the operator **modifies its input register** and is not self-adjoint, so it must implement `dag()` explicitly
([`Add_UInt_UInt_InPlace`](operators.md#op-add-uint-uint-inplace), [`ShiftLeft_InPlace`](operators.md#op-shift-inplace)).

An arithmetic operator without the `_InPlace` suffix ⟺ **XOR output** (out-of-place): the result is XOR-ed into a separate output register,
which makes it automatically unitary and self-adjoint ([`Add_UInt_UInt`](operators.md#op-add-uint-uint), [`Mult_UInt_ConstUInt`](operators.md#op-mult-uint-constuint)).

This dichotomy is how [the operator constraint table in `operators.md`](operators.md#operators-classification) is expressed at the naming level; the two must be kept in sync.

(naming-inverse)=

## 7. Inversion Principles

- **Dagger first**: the inverse of a non-self-adjoint operator is always expressed via `dag()` in preference; no separate class is created for it.
- Legal exception 1: **the `Inverse<Family>` prefix morpheme** — a separate inverse class is allowed only when `dag()` is genuinely unavailable or prohibitively costly,
  named with the leading `Inverse` (`InverseQFT`).
- Legal exception 2: **paired classes that are each other's dagger** — `ShiftLeft_InPlace` / [`ShiftRight_InPlace`](https://github.com/IAI-USTC-Quantum/SparQSim/blob/main/SparQ/include/quantum_arithmetic.h#L301)
  are each other's inverse; this is the established pattern under circular-shift semantics, a documented exemption.
- [`QFT`](https://github.com/IAI-USTC-Quantum/SparQSim/blob/main/SparQ/include/qft.h#L16) has provided `dag()` since the first rename batch (the implementation delegates to the inverse transform).

(naming-gates)=

## 8. Gate-Family Rules

- Fixed single-qubit gates: **`<Axis>_Bool`**, without the `gate` infix — [`X_Bool`](https://github.com/IAI-USTC-Quantum/SparQSim/blob/main/SparQ/include/basic_gates.h#L313), [`Y_Bool`](https://github.com/IAI-USTC-Quantum/SparQSim/blob/main/SparQ/include/basic_gates.h#L370), [`Z_Bool`](https://github.com/IAI-USTC-Quantum/SparQSim/blob/main/SparQ/include/basic_gates.h#L390),
  [`S_Bool`](https://github.com/IAI-USTC-Quantum/SparQSim/blob/main/SparQ/include/basic_gates.h#L411), [`T_Bool`](https://github.com/IAI-USTC-Quantum/SparQSim/blob/main/SparQ/include/basic_gates.h#L432), [`RX_Bool`](https://github.com/IAI-USTC-Quantum/SparQSim/blob/main/SparQ/include/basic_gates.h#L453), [`RY_Bool`](https://github.com/IAI-USTC-Quantum/SparQSim/blob/main/SparQ/include/basic_gates.h#L505), [`RZ_Bool`](https://github.com/IAI-USTC-Quantum/SparQSim/blob/main/SparQ/include/basic_gates.h#L557), [`SX_Bool`](https://github.com/IAI-USTC-Quantum/SparQSim/blob/main/SparQ/include/basic_gates.h#L613), [`U2_Bool`](https://github.com/IAI-USTC-Quantum/SparQSim/blob/main/SparQ/include/basic_gates.h#L661), [`U3_Bool`](https://github.com/IAI-USTC-Quantum/SparQSim/blob/main/SparQ/include/basic_gates.h#L723).
  (The historical `<Axis>gate_Bool` forms are all deprecated; see the [alias table](#naming-audit-renames).)
- The parameterized carrier base classes `Phase_Bool` (phase e^{iλ}) and `Rot_Bool` (an arbitrary 2×2 unitary matrix) are not fixed gates
  and naturally carry no `gate` infix. Inheritance: Z/S/T ← `Phase_Bool`; RX/RY/SX/U2/U3 ← `Rot_Bool`.
- Gate-family constructors uniformly support both forms `(reg, digit, [params])` and `(reg, [params])` (digit defaults to 0).

(naming-rpn)=

## 9. Compound Verb Pipelines (RPN Reading)

The Family of a multi-operand composite operation is read from right to left, in the order of mathematical application:

- [`Add_Mult_UInt_ConstUInt_InPlace`](https://github.com/IAI-USTC-Quantum/SparQSim/blob/main/SparQ/include/quantum_arithmetic.h#L484) = `res += lhs * const`.
- [`Div_Sqrt_Arccos_UInt_UInt`](https://github.com/IAI-USTC-Quantum/SparQSim/blob/main/SparQ/include/quantum_arithmetic.h#L1066) = `res ^= arccos(√(lhs/rhs)) / 2π`.
- [`Sqrt_Div_Arccos_Int_UInt`](https://github.com/IAI-USTC-Quantum/SparQSim/blob/main/SparQ/include/quantum_arithmetic.h#L1166) = `res ^= arccos(lhs/√rhs) / 2π`.

Every morpheme in the verb chain is an independent mathematical operator; the Slots that follow describe the **raw inputs** of the chain.

(naming-alias)=

## 10. Deprecation and Alias Mechanism

- **Python side**: [`pysparq/__init__.py`](https://github.com/IAI-USTC-Quantum/SparQSim/blob/main/PySparQ/__init__.py) provides the old names via the PEP 562 module-level `__getattr__`,
  emitting a `DeprecationWarning` on access and resolving to the new name. The alias table corresponds one-to-one with the rename master table.
- **C++ side**: the header file where a rename happened provides
  `[[deprecated("use NewName")]] using OldName = NewName;` inside the same namespace.
- **Removal policy**: aliases are kept until the next major version.
- From the external-consumer perspective: old names that external dependencies rely on, as recorded in
  [`PySparQ/consumer_runtime_inventory.json`](https://github.com/IAI-USTC-Quantum/SparQSim/blob/main/PySparQ/consumer_runtime_inventory.json) in the SparQSim repository (e.g. `Xgate_Bool`), remain usable
  during the alias period; downstream projects should migrate to the new names within that period.

(naming-audit)=

## 11. Audit Cross-Reference Table

Disposition meanings: **renamed** (executed in the first batch; the old name is a deprecated alias) / **compliant** (already conforms to the specification) /
**exempt** (label-free under the layering or disambiguation principles) / **legacy** (a recorded deviation, not addressed for now).

(naming-audit-renames)=

### 11.1 First-Batch Renames (19 Items)

| # | Old Name | New Name | Rationale |
|---|---|---|---|
| 1 | `Xgate_Bool` | `X_Bool` | gate family drops the `gate` infix ([Section 8](#naming-gates)); externally depended upon, carried by the alias |
| 2 | `Ygate_Bool` | `Y_Bool` | same as above |
| 3 | `Zgate_Bool` | `Z_Bool` | same as above |
| 4 | `Sgate_Bool` | `S_Bool` | same as above |
| 5 | `Tgate_Bool` | `T_Bool` | same as above |
| 6 | `RXgate_Bool` | `RX_Bool` | same as above |
| 7 | `RYgate_Bool` | `RY_Bool` | same as above |
| 8 | `RZgate_Bool` | `RZ_Bool` | same as above |
| 9 | `SXgate_Bool` | `SX_Bool` | same as above |
| 10 | `U2gate_Bool` | `U2_Bool` | same as above |
| 11 | `U3gate_Bool` | `U3_Bool` | same as above |
| 12 | `inverseQFT` | `InverseQFT` | the only lowercase-initial class name ([Section 2](#naming-casing)); `QFT` also gained `dag()` ([Section 7](#naming-inverse)) |
| 13 | `GlobalPhase_Int` | [`GlobalPhase`](https://github.com/IAI-USTC-Quantum/SparQSim/blob/main/SparQ/include/parallel_phase_operations.h#L188) | lying label: the parameter is a complex constant, there is no integer register; global class exempt from labels ([Section 4](#naming-slots)) |
| 14 | `AddAssign_AnyInt_AnyInt_InPlace` | [`Add_AnyInt_AnyInt_InPlace`](https://github.com/IAI-USTC-Quantum/SparQSim/blob/main/SparQ/include/quantum_arithmetic.h#L2643) | two syntaxes for one concept, unified into the `Add_` family ([Sections 3, 9](#naming-grammar)) |
| 15 | `Div_Sqrt_Arccos_Int_Int` | `Div_Sqrt_Arccos_UInt_UInt` | lying label: the constructor checks UInt, UInt |
| 16 | `Sqrt_Div_Arccos_Int_Int` | `Sqrt_Div_Arccos_Int_UInt` | lying label: the constructor checks SInt, UInt |
| 17 | `Hadamard_PartialQubit` | `Hadamard_Partial` | variant-suffix rule ([Section 3](#naming-grammar)) |
| 18 | `CondRot_General_Bool_fast` | [`CondRot_General_Bool_Fast`](https://github.com/IAI-USTC-Quantum/SparQSim/blob/main/SparQ/include/condrot.h#L135) | variant suffix in PascalCase ([Section 3](#naming-grammar)); C++-only, not bound to Python; the alias `CondRot_General_Bool` stays unchanged |
| 19 | `QuantumBinarySearchFast` (namespace CKS) | [`QuantumBinarySearch_Fast`](https://github.com/IAI-USTC-Quantum/SparQSim/blob/main/SparQ_Algorithm/include/hamiltonian_simulation.h#L751) | variant-suffix rule; the base name `QuantumBinarySearch` is untouched |

Verified and **not renamed**: the [`GetRotateAngle_Int_Int`](https://github.com/IAI-USTC-Quantum/SparQSim/blob/main/SparQ/include/quantum_arithmetic.h#L1266) implementation interprets the value as signed via `get_complement`,
so `_Int_Int` is honest; kept as compliant.

### 11.2 Idempotent-Operator Layer — Compliant/Exempt List

- Arithmetic ([quantum_arithmetic.h](https://github.com/IAI-USTC-Quantum/SparQSim/blob/main/SparQ/include/quantum_arithmetic.h)): `FlipBools` (exempt), `Swap_Bool_Bool`,
  `ShiftLeft_InPlace`, `ShiftRight_InPlace`, `Mult_UInt_ConstUInt`,
  `Add_Mult_UInt_ConstUInt_InPlace`, [`Mod_Mult_UInt_ConstUInt_InPlace`](https://github.com/IAI-USTC-Quantum/SparQSim/blob/main/SparQ/include/quantum_arithmetic.h#L583),
  `Add_UInt_UInt`, `Add_UInt_UInt_InPlace`, [`Add_UInt_ConstUInt`](https://github.com/IAI-USTC-Quantum/SparQSim/blob/main/SparQ/include/quantum_arithmetic.h#L875),
  [`Add_ConstUInt_InPlace`](https://github.com/IAI-USTC-Quantum/SparQSim/blob/main/SparQ/include/quantum_arithmetic.h#L964), `GetRotateAngle_Int_Int`, `Assign` (exempt),
  `Compare_UInt_UInt`, [`Less_UInt_UInt`](https://github.com/IAI-USTC-Quantum/SparQSim/blob/main/SparQ/include/quantum_arithmetic.h#L2958), `Swap_General_General`,
  [`GetMid_UInt_UInt`](https://github.com/IAI-USTC-Quantum/SparQSim/blob/main/SparQ/include/quantum_arithmetic.h#L3145), `CustomArithmetic` (exempt) — compliant.
- Gate family ([basic_gates.h](https://github.com/IAI-USTC-Quantum/SparQSim/blob/main/SparQ/include/basic_gates.h)): `Phase_Bool`, `Rot_Bool`, plus the 11 renamed fixed gates — compliant.
- Hadamard ([hadamard.h](https://github.com/IAI-USTC-Quantum/SparQSim/blob/main/SparQ/include/hadamard.h)): [`Hadamard_Int`](https://github.com/IAI-USTC-Quantum/SparQSim/blob/main/SparQ/include/hadamard.h#L23), [`Hadamard_Int_Full`](https://github.com/IAI-USTC-Quantum/SparQSim/blob/main/SparQ/include/hadamard.h#L100), `Hadamard_Bool` — compliant.
- QFT ([qft.h](https://github.com/IAI-USTC-Quantum/SparQSim/blob/main/SparQ/include/qft.h)): `QFT`, [`QFT_Full`](https://github.com/IAI-USTC-Quantum/SparQSim/blob/main/SparQ/include/qft.h#L141) — compliant.
- Conditional rotation ([condrot.h](https://github.com/IAI-USTC-Quantum/SparQSim/blob/main/SparQ/include/condrot.h)): [`CondRot_Rational_Bool`](https://github.com/IAI-USTC-Quantum/SparQSim/blob/main/SparQ/include/condrot.h#L66), `CondRot_Fixed_Bool` — compliant.
- Phase and reflection ([parallel_phase_operations.h](https://github.com/IAI-USTC-Quantum/SparQSim/blob/main/SparQ/include/parallel_phase_operations.h)): [`ZeroConditionalPhaseFlip`](https://github.com/IAI-USTC-Quantum/SparQSim/blob/main/SparQ/include/parallel_phase_operations.h#L21),
  [`RangeConditionalPhaseFlip`](https://github.com/IAI-USTC-Quantum/SparQSim/blob/main/SparQ/include/parallel_phase_operations.h#L72), [`Reflection_Bool`](https://github.com/IAI-USTC-Quantum/SparQSim/blob/main/SparQ/include/parallel_phase_operations.h#L112) — compliant.
- Rotation and state preparation ([rot.h](https://github.com/IAI-USTC-Quantum/SparQSim/blob/main/SparQ/include/rot.h)): [`Rot_GeneralUnitary`](https://github.com/IAI-USTC-Quantum/SparQSim/blob/main/SparQ/include/rot.h#L22), [`Rot_GeneralStatePrep`](https://github.com/IAI-USTC-Quantum/SparQSim/blob/main/SparQ/include/rot.h#L129) — compliant
  (here `General` is a Family morpheme meaning "arbitrary dimension / arbitrary target", not a TypeTag — do not confuse them).
- System operators ([system_operations.h](https://github.com/IAI-USTC-Quantum/SparQSim/blob/main/SparQ/include/system_operations.h)): [`ClearZero`](https://github.com/IAI-USTC-Quantum/SparQSim/blob/main/SparQ/include/system_operations.h#L574) — exempt (global class).
- Dark magic ([dark_magic.h](https://github.com/IAI-USTC-Quantum/SparQSim/blob/main/SparQ/include/dark_magic.h)): [`Normalize`](https://github.com/IAI-USTC-Quantum/SparQSim/blob/main/SparQ/include/dark_magic.h#L21), [`Init_Unsafe`](https://github.com/IAI-USTC-Quantum/SparQSim/blob/main/SparQ/include/dark_magic.h#L45) — exempt (global classes;
  the `_Unsafe` suffix marks a non-physical operation and is retained).

### 11.3 Other Layers

State-management layer, measurement and readout layer, debug and verification layer, comparison functor layer, free-function layer:
all are handled under the layered rules of [Section 1](#naming-scope); **zero renames** in the first batch.

(naming-legacy)=

### 11.4 Recorded Legacy Deviations (Not Addressed; for Awareness Only)

- **The QRAM family is exempt as a whole**: the `_qutrit` snake-case disambiguation suffix of [`QRAMCircuit_qutrit`](https://github.com/IAI-USTC-Quantum/SparQSim/blob/main/PySparQ/core.cpp#L555) and
  the underscore-free variant suffix of [`QRAMLoadFast`](https://github.com/IAI-USTC-Quantum/SparQSim/blob/main/SparQ/include/qram.h#L141) both deviate from the grammar of [Section 3](#naming-grammar); by decision the **status quo is kept**.
- **Ghost reference**: `QRAMLoad_Qubit`, mentioned in a comment in [`SparQ/include/qram.h`](https://github.com/IAI-USTC-Quantum/SparQSim/blob/main/SparQ/include/qram.h), does not exist in the code base.
- The Hamiltonian-simulation class name `T` in `SparQ_Algorithm` collides conceptually with the gate `T_Bool` (the former is not bound to Python).
- The `Sort*` family ([`SortExceptKey`](https://github.com/IAI-USTC-Quantum/SparQSim/blob/main/SparQ/include/sort_state.h#L21) etc.) consists of simulator-internal interference-optimization operations; their naming semantics
  (Except/Key/Bit/Unconditional/ByAmplitude) are as recorded here and do not follow the arithmetic grammar.
- Two classes named `QRAMCircuit` live in the `qram_qutrit` / `qram_qubit` namespaces respectively, disambiguated by the namespace.

## Maintenance Obligations

When adding a new operator you must: name it according to [Sections 3–9](#naming-grammar); if a new TypeTag/Modifier/Variant morpheme is introduced,
revise this specification first; if renaming, provide aliases on both sides per [Section 10](#naming-alias) and update this table and [`operators.md`](operators.md).

## Related Pages

- [Quantum Arithmetic Operator Constraints](operators.md) — operator semantics, width and truncation conventions, unitarity guarantees
- [Quickstart](quickstart.md) — minimal runnable C++/Python examples
