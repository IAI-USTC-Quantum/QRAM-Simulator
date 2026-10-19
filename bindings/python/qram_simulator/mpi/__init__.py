"""qram_simulator.mpi — MPI ensemble parallel execution (trajectory level).

Single-node multi-process CPU parallelism for the noisy QRAM ensemble
workload (the ``T2-MPI`` benchmark family):

- ranks shard the Monte-Carlo trajectory ensemble cyclically
  (``[rank::size]``);
- every trajectory is seeded deterministically via
  ``set_seed(seed + trajectory_index)`` before ``run_normal()``, so the
  per-trajectory outcome is independent of the process count and of the
  sharding;
- rank 0 gathers the per-rank partials and aggregates them into exactly
  the same artifacts a serial run of the same total trajectory count
  would produce (per-trajectory samples identical; the histogram may
  differ only by floating-point summation order, ~1e-16 relative).

Honest scope: this is trajectory/ensemble-level parallelism over CPU
processes (the Python bindings are CPU-only). It is not multi-GPU and
not a distributed sparse-state partitioning.

Usage under MPI::

    mpirun -np 4 python bench_script.py --mpi   # each rank enters the script

Without a process manager ``world_comm()`` returns :class:`SerialComm`
(the same code path runs serially), which keeps the module importable and
testable in environments without an MPI runtime (e.g. CI). This guard is
also required for correctness: importing mpi4py outside ``mpirun``
triggers OpenMPI's *singleton init*, which ``setenv()``s ``OMPI_*``/``PMI*``
variables at C level (invisible to ``os.environ``); any ``mpirun`` child
launched later inherits them and aborts with a silent exit code 1.
"""
from __future__ import annotations

import time
from typing import Dict, List, Optional, Sequence

__all__ = [
    "SerialComm",
    "world_comm",
    "shard_indices",
    "run_trajectories",
    "aggregate",
    "run_ensemble",
    "mpi_metadata",
]

_INPUT_MODES = ("zerobus", "uniform", "random")

# environment variables a process manager sets on every rank — used to
# detect whether we were launched by mpirun/mpiexec (OpenMPI / MPICH / PMIx)
_LAUNCHER_ENV_KEYS = ("OMPI_COMM_WORLD_SIZE", "PMI_SIZE", "PMI_RANK", "PMIX_RANK")


class SerialComm:
    """Drop-in stand-in for ``mpi4py.MPI.COMM_WORLD`` with size == 1."""

    rank = 0
    size = 1

    def barrier(self) -> None:  # noqa: D102
        return None

    def bcast(self, obj, root: int = 0):  # noqa: D102
        return obj

    def gather(self, obj, root: int = 0):  # noqa: D102
        return [obj]

    def allgather(self, obj):  # noqa: D102
        return [obj]


def world_comm():
    """Return ``mpi4py.MPI.COMM_WORLD`` under a process manager, else :class:`SerialComm`.

    Launched under ``mpirun``/``mpiexec`` the launcher sets rank/size
    environment variables on every rank — only then is mpi4py imported and
    ``COMM_WORLD`` returned. Otherwise a :class:`SerialComm` is returned so
    the exact same code path runs serially *without* importing mpi4py:
    importing it outside a process manager triggers OpenMPI's singleton
    init, whose C-level ``setenv()`` side effects (invisible to
    ``os.environ``) silently break any ``mpirun`` child spawned later from
    this process (exit code 1, no diagnostics).
    """
    import os  # noqa: PLC0415

    if not any(key in os.environ for key in _LAUNCHER_ENV_KEYS):
        return SerialComm()
    try:
        from mpi4py import MPI  # noqa: PLC0415
    except ImportError:
        return SerialComm()
    return MPI.COMM_WORLD


def shard_indices(runs: int, size: int, rank: int) -> List[int]:
    """Cyclic trajectory shard ``[rank::size]`` (balances per-trajectory cost)."""
    if runs < 0:
        raise ValueError(f"runs must be >= 0, got {runs}")
    if not 0 <= rank < max(size, 1):
        raise ValueError(f"rank {rank} out of range for size {size}")
    return list(range(rank, runs, size))


def _norm_noise(qs, noise: Dict[object, float]) -> Dict[object, float]:
    """Accept OperationType members, their names or int values as keys."""
    out: Dict[object, float] = {}
    for op, p in noise.items():
        if isinstance(op, qs.OperationType):
            key = op
        elif isinstance(op, str):
            key = getattr(qs.OperationType, op)
        else:
            key = qs.OperationType(int(op))
        out[key] = float(p)
    return out


def run_trajectories(qs, *, indices: Sequence[int], seed: int, noise: Dict[object, float],
                     depth: int, data_size: int,
                     memory: Optional[Sequence[int]] = None,
                     input_mode: str = "zerobus", n_inputs: int = 0) -> dict:
    """Run the bench-faithful pruned trajectory loop over ``indices``.

    Mirrors the reference serial loop of the T2 benchmark
    (``run_normal -> sample_and_get_fidelity -> get_output_distribution
    -> get_fidelity_conventions``), with the single addition of a
    deterministic per-trajectory reseed ``set_seed(seed + index)``.

    ``qs`` is the ``qram_simulator`` module (injected so tests and the
    source-tree fallback loader can pass whichever import succeeded).
    """
    import numpy as np  # noqa: PLC0415

    if input_mode not in _INPUT_MODES:
        raise ValueError(f"input_mode must be one of {_INPUT_MODES}, got {input_mode!r}")
    if memory is not None and max(memory, default=0) >= (1 << data_size):
        # the bus register is data_size wide; larger values produce out-of-range
        # "addr:bus" keys in get_output_distribution()
        raise ValueError(
            f"memory values must be < 2^{data_size} (bus width); "
            f"max={max(memory)}")

    qram = qs.QRAMCircuitQubit(depth, data_size)
    if memory is not None:
        qram.set_memory([int(x) for x in memory])
    else:
        qram.set_memory_random()
    qram.set_noise_models(_norm_noise(qs, noise))
    if input_mode == "zerobus":
        qram.set_input_zerobus()
    elif input_mode == "uniform":
        qram.set_input_uniform(int(n_inputs))
    else:
        qram.set_input_random(int(n_inputs))

    hist = np.zeros((1 << depth, 1 << data_size), dtype=np.float64)
    overlap: List[float] = []
    nopost: List[float] = []
    incoh: List[float] = []
    post: List[Optional[float]] = []
    run_times: List[float] = []
    sample_failures = 0

    t_loop = time.perf_counter()
    for idx in indices:
        qs.set_seed(int(seed) + int(idx))   # deterministic per-trajectory stream
        t0 = time.perf_counter()
        qram.run_normal()
        dt = time.perf_counter() - t0
        try:
            post.append(float(qram.sample_and_get_fidelity()))
        except RuntimeError:
            post.append(None)
            sample_failures += 1
        dist = qram.get_output_distribution()      # conditional dist of this sample
        conv = qram.get_fidelity_conventions()     # overlaps on the posterior trajectory
        for k, v in dist.items():
            a, b = k.split(":")
            hist[int(a), int(b)] += v
        overlap.append(float(conv["overlap_fid"]))
        nopost.append(float(conv["fid_nopost"]))
        incoh.append(float(conv["fid_incoh"]))
        run_times.append(dt)
    wall_loop = time.perf_counter() - t_loop

    return {
        "indices": [int(i) for i in indices],
        "hist": hist,
        "overlap_fid": overlap,
        "fid_nopost": nopost,
        "fid_incoh": incoh,
        "fid_post": post,
        "run_times_s": run_times,
        "sample_failures": sample_failures,
        "wall_loop_s": wall_loop,
        "qubit_num": qram.get_qubit_num(),
    }


def aggregate(partials: List[dict], runs: int) -> dict:
    """Deterministically merge per-rank partials into ensemble outputs.

    Per-trajectory samples are re-ordered by trajectory index, so they are
    bit-identical across shardings; the histogram is summed over partials
    in rank order (a serial global-order sum differs only by float
    associativity, ~1e-16 relative).
    """
    import numpy as np  # noqa: PLC0415

    if not partials:
        raise ValueError("no partials to aggregate")

    rows = []   # (index, overlap, nopost, incoh, post, run_time)
    for p in sorted(partials, key=lambda p: p.get("rank", 0)):
        rows.extend(zip(p["indices"], p["overlap_fid"], p["fid_nopost"],
                        p["fid_incoh"], p["fid_post"], p["run_times_s"]))
    rows.sort(key=lambda r: r[0])
    order = [r[0] for r in rows]
    if order != list(range(runs)):
        raise ValueError(f"shards do not partition [0, {runs})")

    hist = np.zeros_like(partials[0]["hist"], dtype=np.float64)
    for p in sorted(partials, key=lambda p: p.get("rank", 0)):
        hist += p["hist"]
    hist /= runs

    return {
        "traj_index": order,
        "hist": hist,
        "overlap_fid": [r[1] for r in rows],
        "fid_nopost": [r[2] for r in rows],
        "fid_incoh": [r[3] for r in rows],
        "fid_post": [r[4] for r in rows],
        "run_times_s": [r[5] for r in rows],
        "sample_failures": sum(int(p.get("sample_failures", 0)) for p in partials),
        "wall_loop_s_per_rank": {int(p.get("rank", 0)): float(p["wall_loop_s"])
                                 for p in partials},
        "qubit_num": partials[0].get("qubit_num"),
    }


def mpi_metadata(comm) -> dict:
    """Runtime facts for the parallel-execution record (ranks + GPUs, honestly)."""
    impl = "serial (not launched under a process manager)"
    if not isinstance(comm, SerialComm):
        try:
            from mpi4py import MPI, __version__  # noqa: PLC0415
            impl = (f"mpi4py {__version__} / "
                    f"{MPI.Get_library_version().splitlines()[0].strip()}")
        except ImportError:
            impl = "mpi4py not importable"
    return {
        "nranks": int(getattr(comm, "size", 1)),
        "gpu_per_rank": 0,   # CPU-only bindings — recorded as zero, never inferred
        "gpu_note": "qram_simulator Python bindings are CPU-only; "
                    "MPI ranks are CPU processes",
        "implementation": impl,
        "sharding": "cyclic [rank::size]",
        "seed_scheme": "per-trajectory set_seed(seed + trajectory_index)",
    }


def run_ensemble(qs, *, depth: int, data_size: int, noise: Dict[object, float],
                 runs: int, seed: int,
                 memory: Optional[Sequence[int]] = None,
                 input_mode: str = "zerobus", n_inputs: int = 0,
                 comm=None) -> Optional[dict]:
    """Execute the whole ensemble under ``comm``; aggregate on rank 0.

    Returns the :func:`aggregate` result plus an ``mpi`` metadata block on
    rank 0 (``None`` on other ranks). ``noise`` maps ``OperationType``
    members (or their names / int values) to channel probabilities. When
    ``memory`` is None, rank 0 draws it via ``set_memory_random`` and
    broadcasts it, so every rank simulates the same memory.
    """
    comm = world_comm() if comm is None else comm
    rank, size = comm.rank, comm.size
    import numpy as np  # noqa: PLC0415

    if memory is None:
        if rank == 0:
            qs.set_seed(int(seed))   # mirror the serial bench: seed before memory draw
            probe = qs.QRAMCircuitQubit(depth, data_size)
            probe.set_memory_random()
            memory = [int(x) for x in probe.get_memory()]
        memory = comm.bcast(memory, root=0)
    elif len(memory) != (1 << depth):
        raise ValueError(f"memory length {len(memory)} != 2^{depth}")

    if input_mode not in _INPUT_MODES:
        raise ValueError(f"input_mode must be one of {_INPUT_MODES}, got {input_mode!r}")
    if input_mode in ("uniform", "random"):
        n_inputs = int(n_inputs) or (1 << depth)

    partial = run_trajectories(
        qs, indices=shard_indices(runs, size, rank), seed=seed, noise=noise,
        depth=depth, data_size=data_size, memory=memory,
        input_mode=input_mode, n_inputs=n_inputs)
    partial["rank"] = rank

    gathered = comm.gather(partial, root=0)
    if rank != 0:
        return None

    result = aggregate(gathered, runs)
    result["mpi"] = mpi_metadata(comm)
    result["mpi"]["runs_per_rank"] = {int(p["rank"]): len(p["indices"])
                                      for p in gathered}
    result["memory"] = np.asarray(memory, dtype=np.uint64)
    return result
