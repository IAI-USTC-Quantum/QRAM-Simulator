# Tests for qram_simulator.mpi (ensemble-level MPI parallel execution).
#
# Layer 1 (pure logic, no C++ core): shard / aggregate determinism —
#         runs everywhere, including CI runners without MPI.
# Layer 2 (integration, needs the built bindings): the bench-faithful
#         trajectory loop is rank-count invariant — SerialComm (size 1)
#         vs simulated multi-rank shardings produce bit-identical
#         per-trajectory samples and histograms up to float summation order.
# Layer 3 (real MPI): self-contained subprocess launched via mpirun;
#         skipped unless mpirun + mpi4py exist (CI: skipped, a800: runs).

import os
import shutil
import subprocess
import sys
from pathlib import Path

import numpy as np
import pytest

try:
    from qram_simulator.mpi import SerialComm, aggregate, shard_indices
except ImportError:  # bindings not built in this interpreter (e.g. no Python
    # headers) — the mpi subpackage itself is pure python and has no parent
    # package dependency, so load it directly from the source tree
    import importlib.util

    _mpi_src = Path(__file__).resolve().parents[1] / "qram_simulator" / "mpi" / "__init__.py"
    _spec = importlib.util.spec_from_file_location("qs_mpi_standalone", _mpi_src)
    _mod = importlib.util.module_from_spec(_spec)
    _spec.loader.exec_module(_mod)
    SerialComm, aggregate, shard_indices = _mod.SerialComm, _mod.aggregate, _mod.shard_indices

# ---------------------------------------------------------------------------
# Layer 1: pure logic
# ---------------------------------------------------------------------------


def test_shard_indices_partition_and_shape():
    for size in (1, 2, 3, 4, 8):
        shards = [shard_indices(100, size, r) for r in range(size)]
        assert sorted(i for s in shards for i in s) == list(range(100))
        for r, s in enumerate(shards):  # cyclic: own index mod size == rank
            assert all(i % size == r for i in s)


def test_shard_indices_more_ranks_than_runs():
    # runs < size: trailing ranks simply get an empty shard
    shards = [shard_indices(3, 8, r) for r in range(8)]
    assert sorted(i for s in shards for i in s) == [0, 1, 2]


def test_shard_indices_validation():
    with pytest.raises(ValueError):
        shard_indices(-1, 2, 0)
    with pytest.raises(ValueError):
        shard_indices(10, 4, 4)
    with pytest.raises(ValueError):
        shard_indices(10, 4, -1)


def _synthetic_partials(runs: int, size: int, seed: int = 7):
    rng = np.random.default_rng(seed)
    vals = {i: (float(rng.random()), float(rng.random())) for i in range(runs)}
    partials = []
    for r in range(size):
        idx = shard_indices(runs, size, r)
        hist = np.zeros((8, 2))
        ov, npo, inh, po, rt = [], [], [], [], []
        for i in idx:
            a, b = vals[i]
            ov.append(a)
            npo.append(a + 1.0)
            inh.append(a + 2.0)
            po.append(None if i % 7 == 0 else b)
            rt.append(0.5)
            hist[i % 8, (i * 3) % 2] += b
        partials.append({
            "rank": r, "indices": idx, "hist": hist,
            "overlap_fid": ov, "fid_nopost": npo, "fid_incoh": inh,
            "fid_post": po, "run_times_s": rt,
            "sample_failures": sum(1 for i in idx if i % 7 == 0),
            "wall_loop_s": 1.0, "qubit_num": 4,
        })
    return partials


def test_aggregate_identical_across_shardings():
    ref = aggregate(_synthetic_partials(37, 1), 37)
    assert ref["traj_index"] == list(range(37))
    for size in (2, 3, 4, 8):
        agg = aggregate(_synthetic_partials(37, size), 37)
        assert agg["traj_index"] == list(range(37))
        for field in ("overlap_fid", "fid_nopost", "fid_incoh", "fid_post",
                      "run_times_s"):
            assert agg[field] == ref[field]  # bit-identical (canonical index order)
        assert agg["sample_failures"] == ref["sample_failures"]
        np.testing.assert_allclose(agg["hist"], ref["hist"], atol=1e-15)


def test_aggregate_rejects_incomplete_partition():
    partials = _synthetic_partials(10, 2)
    partials[1]["indices"] = partials[1]["indices"][:-1]  # drop one trajectory
    with pytest.raises(ValueError):
        aggregate(partials, 10)


def test_serial_comm_semantics():
    comm = SerialComm()
    assert (comm.rank, comm.size) == (0, 1)
    assert comm.gather("x") == ["x"]
    assert comm.bcast("y") == "y"
    assert comm.allgather(3) == [3]
    comm.barrier()  # no-op, must not raise


# ---------------------------------------------------------------------------
# Layer 2: integration with the built bindings
# ---------------------------------------------------------------------------


@pytest.fixture(scope="module")
def qs():
    return pytest.importorskip("qram_simulator", reason="bindings not built/installed")


def _run_sharded(qs, depth: int, runs: int, size: int, seed: int):
    from qram_simulator.mpi import run_trajectories

    memory = [i % 2 for i in range(1 << depth)]   # bus width 1: values in {0,1}
    partials = []
    for r in range(size):
        p = run_trajectories(
            qs, indices=shard_indices(runs, size, r), seed=seed,
            noise={qs.OperationType.PhaseFlip: 0.001},
            depth=depth, data_size=1,
            memory=memory, input_mode="zerobus")
        p["rank"] = r
        partials.append(p)
    return aggregate(partials, runs)


@pytest.mark.parametrize("size", [2, 3, 4])
def test_ensemble_rank_count_invariance(qs, size):
    depth, runs, seed = 4, 12, 1234
    ref = _run_sharded(qs, depth, runs, size=1, seed=seed)
    agg = _run_sharded(qs, depth, runs, size=size, seed=seed)
    # per-trajectory samples: bit-identical regardless of sharding
    for field in ("overlap_fid", "fid_nopost", "fid_incoh", "fid_post"):
        assert agg[field] == ref[field]
    assert agg["sample_failures"] == ref["sample_failures"]
    np.testing.assert_allclose(agg["hist"], ref["hist"], atol=1e-12)
    assert np.allclose(agg["hist"].sum(), ref["hist"].sum())  # survival matches


def test_run_ensemble_serial_comm(qs):
    from qram_simulator.mpi import run_ensemble

    depth, runs, seed = 4, 9, 99
    memory = [i % 2 for i in range(1 << depth)]
    res = run_ensemble(qs, depth=depth, data_size=1,
                       noise={"PhaseFlip": 0.001}, runs=runs, seed=seed,
                       memory=memory, comm=SerialComm())
    assert res is not None
    assert res["mpi"]["nranks"] == 1
    assert res["mpi"]["gpu_per_rank"] == 0
    assert res["mpi"]["runs_per_rank"] == {0: runs}
    ref = _run_sharded(qs, depth, runs, size=3, seed=seed)
    assert res["overlap_fid"] == ref["overlap_fid"]
    np.testing.assert_allclose(res["hist"], ref["hist"], atol=1e-12)
    assert res["memory"].tolist() == memory


def test_run_ensemble_memory_draw_is_seeded(qs):
    from qram_simulator.mpi import run_ensemble

    kwargs = dict(depth=3, data_size=1,
                  noise={qs.OperationType.BitFlip: 0.01}, runs=4, seed=5,
                  comm=SerialComm())
    res1 = run_ensemble(qs, **kwargs)          # memory=None -> rank0 draw + bcast
    res2 = run_ensemble(qs, **kwargs)
    assert res1["memory"].shape == (8,)
    assert res1["memory"].tolist() == res2["memory"].tolist()  # reproducible
    assert res1["overlap_fid"] == res2["overlap_fid"]


# ---------------------------------------------------------------------------
# Layer 3: real MPI (self-contained subprocess; skipped without mpirun)
# ---------------------------------------------------------------------------


def test_mpi_subprocess_matches_serial(qs, tmp_path):
    if shutil.which("mpirun") is None:
        pytest.skip("mpirun not available")
    pytest.importorskip("mpi4py", reason="mpi4py not installed")

    depth, runs, seed = 4, 16, 1234
    ref = _run_sharded(qs, depth, runs, size=1, seed=seed)

    child = tmp_path / "mpi_hist.npy"
    script = (
        "import numpy as np, sys\n"
        "import qram_simulator as qs\n"
        "from qram_simulator.mpi import run_ensemble\n"
        f"out = sys.argv[1]\n"
        f"res = run_ensemble(qs, depth={depth}, data_size=1,\n"
        f"                   noise={{'PhaseFlip': 0.001}}, runs={runs}, seed={seed},\n"
        f"                   memory=[i % 2 for i in range(1 << {depth})])\n"
        "if res is not None:\n"
        "    np.save(out, res['hist'])\n"
        "    assert res['mpi']['nranks'] == 4\n"
    )
    base = [sys.executable, "-c", script, str(child)]
    as_root = hasattr(os, "geteuid") and os.geteuid() == 0
    # scrub C-level OMPI_*/PMI* setenv() side effects (OpenMPI singleton init
    # in this process breaks mpirun children with a silent exit 1)
    child_env = {k: v for k, v in os.environ.items()
                 if not k.startswith(("OMPI_", "PMI", "PMIX"))}
    cmd = ["mpirun", *(["--allow-run-as-root"] if as_root else []), "-np", "4", *base]
    proc = subprocess.run(cmd, capture_output=True, text=True, timeout=300,
                          env=child_env)
    if proc.returncode != 0 and "unrecognized" in (proc.stdout + proc.stderr).lower():
        # MPICH-style mpirun rejects OpenMPI's root flag — retry without it
        proc = subprocess.run(["mpirun", "-np", "4", *base],
                              capture_output=True, text=True, timeout=300,
                              env=child_env)
    assert proc.returncode == 0, (
        f"cmd={cmd}\n--- stdout ---\n{proc.stdout[-2000:]}\n--- stderr ---\n{proc.stderr[-2000:]}")
    hist = np.load(child)
    np.testing.assert_allclose(hist, ref["hist"], atol=1e-12)
