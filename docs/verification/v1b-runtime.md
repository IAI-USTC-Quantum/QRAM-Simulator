# V1b consumed-runtime verification

The machine-readable freeze is
`PySparQ/consumer_runtime_inventory.json`. It records QECC.Lang
`b8a5ed81a1867d999cbe11be7441e7afc00109fa` and quantum-cfd
`b64a4fec8c860e531a381a26c4275e0f6865a797`.

## Strict status

| Group | Status | Evidence |
| --- | --- | --- |
| Add/assign/compare | verified | exhaustive 2-bit independent XOR models, nonzero targets, superpositions, controls, both inverse orders |
| In-place add/swaps/bit flips | verified | exhaustive independent permutation models, superpositions, controls, both inverse orders |
| QRAM/QRAMFast | verified | persistent `data ^= memory[address]` model, nonzero data, superposed address, controls, self-inverse, reused handle |
| Control/dagger wrappers | verified | nonzero, all-ones, bit and value controls; positive, negative and multi-register conjunctions; coherent controlled dagger |
| QDA boundary | partial by design | analytic schedule/rotation/QRAM helpers plus focused frozen-C++ walk regressions; no whole-solver claim |
| CKS boundary | partial by design | analytic binomial coefficients and signed/unsigned walk rotations; no whole-solver claim |
| `compile_operator` semantics | excluded | zero matches in QECC.Lang `src/qecc_lang` and the listed supported QFVM native/QDA files |

The QFVM files using `compile_operator` are legacy staged materialization
oracles and are explicitly outside this accepted native/QDA slice. No general
`dynamic_operator` runtime audit is claimed.

## Focused audit

The audit found that native constructors allowed destructive role aliases:
XOR outputs could alias inputs, in-place add could alias its addend, compare
flags could alias, and QRAM address/data could alias. Constructors now reject
these non-unitary/contract-destroying shapes in all build modes. Width/type and
bit-index boundaries remain enforced by the existing constructors.
