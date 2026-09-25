# Circuit-level QRAM baseline (qubit architecture)

An independent, gate-by-gate re-implementation of the **qubit-architecture QRAM
loading circuit** on the [UnifiedQuantum](https://github.com/IAI-USTC-Quantum/UnifiedQuantum)
(`uniqc`) QuTiP density-matrix backend, packaged inside `qram_simulator` as the
semi-quantitative baseline of the symbolic sparse-tree trajectory engine
(`QRAMCircuitQubit`).

- **Scope**: 1-2 tree layers only (`addr_size ∈ {1, 2}`, `data_size ∈ {1, 2}`).
  The density matrix lives on `addr + data + 2*(2^addr - 1)` qubits (9 qubits
  for addr=2/data=1); addr=3 would already need 25. `addr_size = 1` has no
  position-sampled noise (`layer_entangle_max ≡ 0`) and serves as the
  noise-free bridge; `addr_size = 2` is the smallest noisy instance.
- **Noise**: channels mirror the C++ `TimeStep` sampler but are placed on the
  **QRAM routing tree only** (never on the address register or the data bus):
  after the gates of each time slice with entangled-layer count `l`, a channel
  acts on every active position `[0, 2*(2^l-1))` with marginal probability `p`
  (the channel-level counterpart of `nerror ~ Binomial(N, p)` on uniform
  positions). Depolarizing = `{I, X, Z, Y}` mixture; Damping = the channel
  counterpart of the C++ rho-dependent sampling scheme (whole-tree K0 + jumps
  on active positions). Advanced: `simulate_density(mode=...)` also offers the
  textbook TP channels (`"textbook"`, deviates systematically under damping)
  and the no-jump-only survival (`"nojump"`).
- **Schedule provenance**: the gate sequence and per-step entangled-layer
  counts are pulled from the C++ `TimeStep` scheduler through
  `qram_simulator._core` (single source of truth) — the baseline never
  re-derives the schedule, so the noise-free output distribution matches the
  C++ engine bitwise.
- **Encoding**: address `[0, addr)`, bus `[addr, addr+data)`, node `v` →
  `a = addr+data+2v`, `d = addr+data+2v+1`; noise position `pos = 2v+lr`
  (lr: 0 → a, 1 → d). Identical to `Experiments/QRAM/ChannelCorrespondence`
  (whose Stage 0/1 verified the gate translation bitwise).

## New binding surface used by the comparison

`QRAMCircuitQubit` gained the experiment-facing API (see `core_binding.cpp`):

- `set_input_zerobus()` — uniform address superposition with bus = 0 (the
  discriminating input; a uniform bus degenerates the output distribution);
- `get_output_distribution()` — marginal `{"addr:bus": prob}` distribution of
  the current run (sub-normalized under Damping; sum = survival);
- `get_fidelity_conventions()` — the three no-post-selection fidelity
  statistics of the current trajectory: `overlap_fid` (`|⟨ψ_ideal|ψ_traj⟩|²`,
  tree-sensitive — the counterpart of the channel-level `⟨ψ_ideal|ρ|ψ_ideal⟩`),
  `fid_nopost` (tree-insensitive) and `fid_incoh` (branch-projected);
- `TimeStep.layer_entangle_max(step)` and `OperationPack.operations` are now
  exposed so Python can read the exact schedule.

## Usage

```bash
pip install "qram-simulator[baseline]"     # pulls uniqc + qutip
```

```python
from qram_simulator import QRAMCircuitQubit, OperationType, set_seed
from qram_simulator.baseline import CircuitQRAMQubit

memory = [0, 1, 1, 0]

# uniqc side: exact channel ensemble (density matrix, no Monte Carlo)
circuit = CircuitQRAMQubit(2, 1, memory, depolarizing=0.02)
result = circuit.run()
print(result["fidelity"], result["trace"])   # <psi_ideal|rho|psi_ideal>, survival

# QRAM side: R independent noise trajectories, same input and conventions
set_seed(20260925)
qram = QRAMCircuitQubit(2, 1, memory)
qram.set_noise_models({OperationType.Depolarizing: 0.02})
qram.set_input_zerobus()
for _ in range(500):
    qram.run_full()
    dist = qram.get_output_distribution()
    overlap = qram.get_fidelity_conventions()["overlap_fid"]
```

## Packaged comparison experiment

```bash
python -m qram_simulator.baseline.compare --addr 2 --runs 500 --seed 20260925 \
    --outdir results
```

Sweeps depolarizing p ∈ {0.005, 0.02, 0.05, 0.1, 0.3}, damping γ ∈
{0.001, 0.01, 0.05, 0.2} and a mixed configuration, and writes
`results.json` / `results.csv`. What gets compared (identical definitions on
both sides):

| quantity | meaning | QRAM-Simulator side | uniqc side | expected agreement |
|---|---|---|---|---|
| fidelity | loading fidelity \|⟨ψ_ideal\|ψ⟩\|² | 500-trajectory average ± MC error | exact ⟨ψ_ideal\|ρ\|ψ_ideal⟩ | within 3σ MC error |
| F_cls / TVD | (addr,bus) output distribution | trajectory-averaged marginal | channel marginal | ≥ 0.99 / ≤ 0.05 |
| survival | damping survival probability | sum of trajectory norms | trace(ρ) | few % |
| bridge | noise-free output distribution | noise-free `run_full` | ideal circuit | bitwise (< 1e-12) |

The QRAM-side per-trajectory statistics (`fid_nopost`, `fid_incoh`, per-shot
`avg_fidelity`) are recorded in the JSON for completeness — they are
alternative fidelity *conventions* that deliberately differ (see
`Experiments/QRAM/ChannelCorrespondence/README.md`); the channel side matches
`overlap_fid`, which the comparison uses.

Results of the last packaged run: see [results.md](results.md).
