# MPI Ensemble Parallelism

The `qram_simulator.mpi` subpackage provides trajectory-level (ensemble) MPI
parallel execution for the noisy QRAM simulation workload.

## What is parallelized

The noisy-ensemble benchmark workload is a Monte-Carlo loop: run a pruned
noisy trajectory (`run_normal`), sample it, read the conditional output
distribution and the fidelity conventions, repeat `runs` times and average.
Trajectories are statistically independent, which makes the ensemble the
natural parallelization unit:

- **Cyclic rank sharding** — rank `r` executes trajectories `[r::size]`
  (balanced also when per-trajectory cost varies);
- **Deterministic per-trajectory reseeding** — `set_seed(seed + index)` before
  every trajectory, so each trajectory's outcome depends only on the global
  seed and its index, never on the process count or the sharding;
- **Rank-0 aggregation** — per-trajectory samples are re-ordered by trajectory
  index (bit-identical across shardings); the histogram is summed over rank
  partials in rank order, which differs from a serial global-order sum only by
  floating-point associativity (~1e-16 relative).

## Honest scope

This is **single-node multi-process CPU** ensemble parallelism:

- the Python bindings are CPU-only — every MPI rank is a CPU process and
  `gpu_per_rank = 0` is recorded in the run metadata (never inferred);
- it is **not** multi-GPU execution and **not** a distributed sparse-state
  partitioning (the C++ core ships no MPI path; states are never split
  across ranks — only the trajectory ensemble is).

## Usage

```python
import qram_simulator as qs
from qram_simulator.mpi import run_ensemble, world_comm

comm = world_comm()   # mpi4py COMM_WORLD under mpirun; SerialComm otherwise
res = run_ensemble(qs, depth=20, data_size=1,
                   noise={qs.OperationType.PhaseFlip: 1e-5},
                   runs=100, seed=20261105,
                   memory=memory_list,      # None -> rank 0 draws + broadcasts
                   comm=comm)
if res is not None:   # rank 0 only; other ranks return None
    hist = res["hist"]             # normalized (2^depth, 2^data_size) histogram
    fids = res["overlap_fid"]      # per-trajectory samples, index-ordered
    meta = res["mpi"]              # nranks / gpu_per_rank / implementation / ...
```

```bash
mpirun -np 4 python noisy_ensemble.py    # every rank enters the script once
```

`mpi4py` is an optional runtime dependency. When it is missing — or when the
script is launched without a process manager — `world_comm()` returns a
`SerialComm` with `size == 1` and the exact same code path runs serially.
That degenerate mode is what keeps the module importable and CI-testable in
MPI-less environments.

## Reproducibility contract

Given the same `seed`, `runs` and `memory`:

- per-trajectory samples (fidelities, run times, sample failures) are
  **bit-identical** for any process count;
- `memory=None`: rank 0 seeds the global RNG (`set_seed(seed)`) before
  drawing the memory and broadcasts it, so the memory itself is reproducible;
- the aggregate histogram at `np` processes equals the `np = 1` histogram up
  to float summation order.

## Scaling and tests

Strong scaling is near-linear by construction (independent trajectories, no
inter-rank communication before the final gather). Measured points are
recorded with `wall_loop_max_s` (max per-rank trajectory-loop wall time,
excluding memory load and setup) so speedups are not diluted by fixed
overhead; pin `OMP_NUM_THREADS=1` for a pure process-scaling reading.

Tests live in `bindings/python/test/test_mpi_ensemble.py`:

- logic layer (sharding, aggregation determinism) runs everywhere, including
  CI runners without MPI;
- the binding-integration layer runs wherever the built package is installed;
- the `mpirun -np 4` integration test runs only where `mpirun` + `mpi4py`
  exist and is skipped otherwise.
