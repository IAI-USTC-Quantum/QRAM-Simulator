# Wide Sparse Word Queries

The ideal sparse XOR query interface — `qram_simulator.SparseWordMemory` —
serves logical address/data words of **arbitrary width** (65, 129, 256 bits
and beyond). It is deliberately separate from the noisy bucket-brigade
routing of `QRAMCircuitQubit`/`QRAMCircuitQutrit`: those native circuits pack
address+bus into one 64-bit branch index and **reject** wider combinations
with an explicit exception, while the sparse word path carries every value as
an exact arbitrary-precision integer and never truncates.

## The interface

```python
from qram_simulator import SparseWordMemory

memory = SparseWordMemory(address_width=129, data_width=193,
                          cells={(1 << 128) + 5: (1 << 192) + 7})
memory.xor((1 << 128) + 5, data=9)   # classical XOR read, self-inverse
```

- `cells` maps addresses to non-zero words; addresses that map to zero (or
  are absent) read the exact default zero;
- `.xor(address, data=0)` is the ideal classical semantics: `data XOR word`,
  exact at every bit, an involution;
- `.apply_xor(state, address_bits, data_bits, backend, conditions=())`
  executes the **coherent query**: one controlled native X per set data bit,
  conditioned on the merged address/condition pattern.

## Native bit coordinates and the operator protocol

A logical register wider than one machine word spans several *word
registers*. Bit coordinates are `(word_register_name, bit_index)` pairs with
`bit_index` inside one 64-bit machine word; `address_bits[i]` / `data_bits[i]`
name logical bit `i` (little-endian). The backend protocol (frozen with the
SparQSim owner; production execution is SparQSim's native backend, which
installs this repository's `word_memory.py` from its submodule as
`pysparq/_qram_words.py` — the single source):

- `backend.X_Bool(word_register_name, bit_index)` returns an operator that
  applies a native X when called: `operator(state)`;
- `operator.conditioned_by_bit(list_of_coordinates)` adds controls on the
  listed bits reading 1 — implementations may mutate in place, return
  `self`, or return a new operator; all are supported.

`conditions` are `(little_endian_bits, expected_value)` pairs: a stored cell
is queried only on basis states whose listed bits hold `expected_value`. A
zero control that coincides with an address bit **restricts** the query to
cells agreeing with it — it never re-addresses a cell; conflicting
address/condition constraints gate the cell off entirely.

The packaged `qram_simulator.word_backend.ReferenceWordBackend` is a real
exact-statevector executor over named machine words (a dict of amplitudes on
arbitrary-width basis indices) used by the CLI/GUI demo and the test chain.
The shared fixture `bindings/python/test/fixtures/word_query_fixture.json`
(`qram.word-query-fixture.v1`, wide integers as decimal strings) lets
QRAM-Simulator and SparQSim verify both native backends on identical source
data.

## Capacity notions

| Notion | Meaning | Cost |
|---|---|---|
| wide logical word | `address_width`/`data_width` bits, exact | Python `int`, never truncated |
| sparse record | one non-zero `cells` entry | the only stored object |
| nominal address space | `2**address_width` labels (`memory.nominal_address_space`) | a pure bound — nothing is allocated |
| allocatable entity capacity | machine words / qubits the caller's backend provides | owned by the backend |

No entity, qubit or tree node is ever allocated for the nominal address
space; the ideal query path is noise-free and makes no noisy-routing claim.

## CLI and GUI

```bash
qram-simulator words --address-width 129 --data-width 193 --records 8 --inputs 4
```

seeds a sparse wide memory, runs the classical XOR and the executed query
chain (superposed query addresses, alternating phases), self-checks that the
chain matches the classical oracle per basis state and that a second query
restores the initial state (involution), and writes `qramsim.words.v1` JSON
with all wide integers as decimal strings. The GUI exposes the same flow in
its wide-word panel; stale results are invalidated on parameter change.

## Related

The 64-bit boundary of the noisy native path is pinned by tests
(`test_word_memory.py::test_narrow_native_boundary_is_explicit_not_silent`):
`addr_size + data_size > 64` raises at input sampling, and oversized Python
integers raise in the binding instead of truncating. The addressing mitigation noisy
addressing/mitigation simulation reference is the
[`addressing` CLI](https://iai-ustc-quantum.github.io/QRAM-Simulator/en/guide/quickstart.html)
(`qramsim.addressing.v1`).
