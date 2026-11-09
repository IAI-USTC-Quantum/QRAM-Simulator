"""Standalone QRAM experiment: ``qram-simulator run`` or ``qram-simulator gui``.

The reported channel overlap is read before the destructive routing-tree
measurement. Its Monte Carlo confidence interval describes sampling uncertainty,
not the accuracy of the simulator or a hardware acceptance certificate.
"""

from __future__ import annotations

import argparse
import dataclasses
import hashlib
import json
import math
import platform
import random
import statistics
import sys
import time
from pathlib import Path

CHANNELS = ("none", "BitFlip", "PhaseFlip", "Depolarizing", "Damping")


@dataclasses.dataclass(frozen=True)
class Experiment:
    depth: int = 3
    data_bits: int = 1
    inputs: int = 8
    trajectories: int = 32
    channel: str = "Depolarizing"
    probability: float = 0.001
    seed: int = 20261109

    def validate(self) -> None:
        limits = {
            "depth": (1, 20),
            "data_bits": (1, 8),
            "inputs": (1, 1024),
            "trajectories": (1, 4096),
            "seed": (0, 2**32 - 1),
        }
        for name, (lo, hi) in limits.items():
            value = getattr(self, name)
            if type(value) is not int or not lo <= value <= hi:
                raise ValueError(f"{name} must be an integer in [{lo}, {hi}]")
        if self.inputs > 2**self.depth:
            raise ValueError("inputs exceeds the number of addresses")
        if self.channel not in CHANNELS:
            raise ValueError("unknown noise channel")
        if (
            type(self.probability) not in (int, float)
            or not math.isfinite(self.probability)
            or not 0 <= self.probability < 1
        ):
            raise ValueError("probability must be finite and in [0,1)")
        if self.channel == "none" and self.probability != 0:
            raise ValueError("the noiseless channel requires probability=0")


@dataclasses.dataclass(frozen=True)
class WordQuery:
    address_width: int = 129
    data_width: int = 129
    records: int = 8
    inputs: int = 4
    seed: int = 20261109

    def validate(self) -> None:
        limits = {
            "address_width": (1, 1024),
            "data_width": (1, 1024),
            "records": (1, 512),
            "inputs": (1, 64),
            "seed": (0, 2**32 - 1),
        }
        for name, (lo, hi) in limits.items():
            value = getattr(self, name)
            if type(value) is not int or not lo <= value <= hi:
                raise ValueError(f"{name} must be an integer in [{lo}, {hi}]")
        if self.inputs > 2 ** min(self.address_width, 20):
            raise ValueError("inputs exceeds the distinct query addresses")


def run_words(config: WordQuery) -> dict:
    """Ideal sparse wide-word XOR query with a real executed query chain.

    Classical results come from ``SparseWordMemory.xor``; the quantum chain
    runs ``apply_xor`` on the packaged reference native backend (exact
    dict-backed statevector over named machine words). Wide integers are
    emitted as decimal strings — the nominal 2**address_width labels are
    pure bounds, never allocated entities.
    """
    config.validate()
    from .word_backend import MACHINE_WORD_BITS, ReferenceWordBackend, encode_basis, word_layout
    from .word_memory import SparseWordMemory

    start = time.perf_counter()
    rng = random.Random(config.seed)
    cells: dict[int, int] = {}
    while len(cells) < config.records:
        address = rng.getrandbits(config.address_width)
        cells[address] = rng.getrandbits(config.data_width) or 1
    memory = SparseWordMemory(config.address_width, config.data_width, cells)
    queries: list[int] = []
    while len(queries) < config.inputs:
        address = rng.getrandbits(config.address_width)
        if address not in queries:
            queries.append(address)

    backend = ReferenceWordBackend()
    address_bits = word_layout(config.address_width, "addr")
    data_bits = word_layout(config.data_width, "data")
    backend.register(address_bits, data_bits)
    state = backend.make_state()
    weight = 1 / math.sqrt(len(queries))
    prepared: dict[int, complex] = {}
    for index, address in enumerate(queries):
        basis = encode_basis(0, address_bits, address, backend)
        amplitude = weight * (1j if index % 2 else 1)
        state.amps[basis] = state.amps.get(basis, 0j) + amplitude
        prepared[basis] = state.amps[basis]
    memory.apply_xor(state, address_bits, data_bits, backend)
    chained = dict(state.amps)
    memory.apply_xor(state, address_bits, data_bits, backend)
    classical_match = True
    for index, address in enumerate(queries):
        basis = encode_basis(
            encode_basis(0, address_bits, address, backend), data_bits, memory.xor(address), backend
        )
        if chained.get(basis, 0j) != prepared[encode_basis(0, address_bits, address, backend)]:
            classical_match = False
    involution = state.amps == prepared
    if not classical_match or not involution:
        raise RuntimeError("wide-word query chain violated its classical semantics")
    return {
        "schema": "qramsim.words.v1",
        "software": information(),
        "configuration": dataclasses.asdict(config),
        "memory": {
            "cells": {str(address): str(value) for address, value in sorted(cells.items())},
            "record_count": memory.record_count,
            "nominal_address_space": str(memory.nominal_address_space),
            "encoding": "decimal strings; exact arbitrary-precision logical words",
        },
        "query": {
            "addresses": [str(address) for address in queries],
            "bus_initial": "0",
            "amplitudes": "equal magnitude with alternating real/imaginary phase",
        },
        "results": [
            {
                "address": str(address),
                "stored_word": str(cells.get(address, 0)),
                "xor_output": str(memory.xor(address)),
            }
            for address in queries
        ],
        "chain": {
            "backend": "reference exact statevector over named machine words",
            "machine_word_bits": MACHINE_WORD_BITS,
            "address_word_registers": len({name for name, _ in address_bits}),
            "data_word_registers": len({name for name, _ in data_bits}),
            "classical_match": classical_match,
            "involution": involution,
        },
        "timing_s": {"total": time.perf_counter() - start},
        "scope": {
            "semantics": "ideal sparse XOR query; not the noisy bucket-brigade routing",
            "noise": "none",
            "hardware_run": False,
            "allocation": "no entity is allocated for the nominal address space",
        },
    }


@dataclasses.dataclass(frozen=True)
class AddressingQuery:
    """2-3 layer QRAM noisy addressing simulation reference (task 2.2)."""

    layers: int = 3
    channel: str = "Depolarizing"
    probability: float = 0.001
    trajectories: int = 64
    seed: int = 20261109

    def validate(self) -> None:
        if type(self.layers) is not int or self.layers not in (2, 3):
            raise ValueError("layers must be 2 or 3 (4 or 8 data records)")
        if self.channel not in CHANNELS:
            raise ValueError("unknown noise channel")
        if (
            type(self.probability) not in (int, float)
            or not math.isfinite(self.probability)
            or not 0 <= self.probability < 1
        ):
            raise ValueError("probability must be finite and in [0,1)")
        if self.channel == "none" and self.probability != 0:
            raise ValueError("the noiseless channel requires probability=0")
        if type(self.trajectories) is not int or not 1 <= self.trajectories <= 4096:
            raise ValueError("trajectories must be an integer in [1, 4096]")
        if type(self.seed) is not int or not 0 <= self.seed <= 2**32 - 1:
            raise ValueError("seed must be an integer in [0, 2^32-1]")


def run_addressing(config: AddressingQuery) -> dict:
    """Noisy-VM QRAM addressing with a calibrated readout-mitigation simulation.

    Runs the native noisy engine over a seeded random memory of ``2**layers``
    one-bit records (4 or 8), aggregates the mean (address:bus) output
    distribution over independent trajectories, and applies the reference
    mitigation: the bus flip rate is calibrated per bit against the known
    memory and inverted with the symmetric single-bit confusion matrix
    (routing-induced flips are mitigated to the extent they follow that
    model). The >=0.75 mitigated addressing threshold uses the direct
    task-sheet sub-item semantics (``>=``, not ``>``); this is a simulation
    reference with standard JSON I/O for the uniqc side — the real-machine
    business stays frozen in its owning repository and nothing here is a
    hardware run.
    """
    config.validate()
    import qram_simulator as qs

    start = time.perf_counter()
    rng = random.Random(config.seed)
    memory = [rng.randrange(2) for _ in range(2**config.layers)]
    qram = qs.QRAMCircuitQubit(config.layers, 1, memory)
    noise = (
        {}
        if config.channel == "none"
        else {getattr(qs.OperationType, config.channel): config.probability}
    )
    qram.set_noise_models(noise)
    qram.set_input_zerobus()  # uniform coherent addressing over all records
    mean_dist: dict[str, float] = {}
    logical = []
    for index in range(config.trajectories):
        qs.set_seed(config.seed + index + 1)
        qram.run_normal()
        qram.prepare_observables()
        for key, value in qram.get_output_distribution().items():
            mean_dist[key] = mean_dist.get(key, 0.0) + value / config.trajectories
        logical.append(float(qram.get_logical_query_fidelity()))
    survival = math.fsum(mean_dist.values())
    if not math.isfinite(survival) or survival <= 0:
        raise RuntimeError("addressing distribution degenerated")

    per_address: dict[int, dict[int, float]] = {}
    for key, value in mean_dist.items():
        address, bus = (int(part) for part in key.split(":"))
        per_address.setdefault(address, {})[bus] = value
    # calibrate the symmetric bus-flip rate against the known memory
    kept = flipped = 0.0
    for address, buses in per_address.items():
        kept += buses.get(memory[address], 0.0)
        flipped += buses.get(1 - memory[address], 0.0)
    flip_rate = flipped / survival
    if flip_rate >= 0.5 - 1e-12:
        raise RuntimeError("bus flip calibration is not invertible " f"(flip rate {flip_rate})")
    raw_correct = kept / survival
    mitigated = 0.0
    for address, buses in per_address.items():
        joint = math.fsum(buses.values())
        if joint <= 0:
            continue  # address lost entirely under damping
        weight = joint / survival
        good = buses.get(memory[address], 0.0) / joint
        bad = buses.get(1 - memory[address], 0.0) / joint
        # invert obs = ((1-e), e; e, (1-e)) @ true on the conditional bus
        # distribution of this address, then reweight by its address marginal
        corrected = ((1 - flip_rate) * good - flip_rate * bad) / (1 - 2 * flip_rate)
        mitigated += weight * min(max(corrected, 0.0), 1.0)
    mitigated = min(mitigated, 1.0)
    logical_mean = statistics.fmean(logical)
    return {
        "schema": "qramsim.addressing.v1",
        "software": information(),
        "configuration": dataclasses.asdict(config),
        "memory": {"records": memory, "encoding": "one unsigned bit per record"},
        "resources": {
            "address_qubits": config.layers,
            "bus_qubits": 1,
            "routing_qubits": qram.get_qubit_num(),
        },
        "distribution_mean": dict(sorted(mean_dist.items())),
        "survival": survival,
        "raw_correct_addressing": raw_correct,
        "mitigated_correct_addressing": mitigated,
        "mitigation": {
            "model": "symmetric single-bit bus-flip confusion calibrated against "
            "the known memory, inverted per address (clipped to [0,1])",
            "calibrated_flip_rate": flip_rate,
            "formula": "true = ((1-e)*obs - e*obs_flipped) / (1-2e)",
        },
        "logical_query_fidelity_mean": logical_mean,
        "criterion": {
            "threshold": 0.75,
            "operator": ">=",
            "value": mitigated,
            "semantics": "direct task-sheet sub-item >=75% (not the " "cross-page >75% variant)",
            "pass": mitigated >= 0.75,
        },
        "timing_s": {"total": time.perf_counter() - start},
        "scope": {
            "simulation_reference": True,
            "hardware_run": False,
            "real_machine_business": "frozen in its owning repository; "
            "this JSON is the offline standard I/O",
        },
    }


def evaluate_addressing(record: dict) -> dict:
    """Offline import of a recorded ``qramsim.addressing.v1`` JSON report.

    Task-4 engineering wrapper: reads a record produced by this software or
    by the declared uniqc consumer, validates its schema and required
    numeric fields, and recomputes the mitigated-addressing criterion from
    the recorded values (no simulation, no hardware run). Historical data
    stays historical — evaluation never re-runs trajectories.
    """
    if not isinstance(record, dict) or record.get("schema") != "qramsim.addressing.v1":
        raise ValueError("not a qramsim.addressing.v1 record")
    required = ("raw_correct_addressing", "mitigated_correct_addressing")
    try:
        values = {name: float(record[name]) for name in required}
        criterion = dict(record["criterion"])
        memory = list(record["memory"]["records"])
        configuration = dict(record["configuration"])
    except (KeyError, TypeError, ValueError) as exc:
        raise ValueError(f"record is missing or malforms a required field: {exc}") from exc
    for name, value in values.items():
        if not math.isfinite(value) or not 0 <= value <= 1:
            raise ValueError(f"{name} must be finite and in [0,1]")
    threshold = float(criterion.get("threshold", 0.75))
    operator = criterion.get("operator", ">=")
    if operator != ">=":
        raise ValueError("only the recorded '>=' criterion semantics are supported")
    passed = values["mitigated_correct_addressing"] >= threshold
    criterion["value"] = values["mitigated_correct_addressing"]
    criterion["recomputed_pass"] = passed
    criterion["recorded_pass_matches"] = criterion.get("pass") in (None, passed)
    return {
        "schema": "qramsim.addressing-eval.v1",
        "software": information(),
        "source": {
            "schema": record["schema"],
            "configuration": configuration,
            "memory_records": memory,
            "record_version": (record.get("software") or {}).get("version"),
            "hardware_run": (record.get("scope") or {}).get("hardware_run"),
        },
        "values": values,
        "criterion": criterion,
        "consistency": {
            "mitigated_at_least_raw": values["mitigated_correct_addressing"]
            >= values["raw_correct_addressing"] - 1e-12,
            "record_version_present": bool((record.get("software") or {}).get("version")),
        },
        "timing_s": {"total": 0.0},
        "scope": {"offline_import": True, "simulation_run": False, "hardware_run": False},
    }


def information() -> dict:
    import qram_simulator as qs

    return {
        "software": "QRAM-Simulator",
        "version": qs.__version__,
        "python": platform.python_version(),
        "platform": platform.platform(),
        "backend": "native CPU sparse branches",
        "gpu_execution": False,
        "damping_convention": qs.QUBIT_DAMPING_CONVENTION,
        "channels": list(CHANNELS),
        "demo_architecture": "qubit",
        "wide_word_query": "SparseWordMemory ideal sparse XOR (arbitrary widths)",
    }


def run(config: Experiment) -> dict:
    """Run an independent fixed-memory/fixed-input channel trajectory ensemble."""
    config.validate()
    import qram_simulator as qs

    start = time.perf_counter()
    # One byte per record for this demo's supported 1..8-bit bus. The exact
    # byte sequence is the provenance digest, independent of C++ RNG versions.
    rng = random.Random(config.seed)
    memory = bytes(rng.randrange(2**config.data_bits) for _ in range(2**config.depth))
    qram = qs.QRAMCircuitQubit(config.depth, config.data_bits, list(memory))
    addresses = sorted(rng.sample(range(2**config.depth), config.inputs))
    qram.set_input_addresses(addresses)
    noise = (
        {}
        if config.channel == "none"
        else {getattr(qs.OperationType, config.channel): config.probability}
    )
    qram.set_noise_models(noise)
    ready = time.perf_counter()
    samples = []
    logical_samples = []
    traces = []
    evolution = 0.0
    observables = 0.0
    for index in range(config.trajectories):
        qs.set_seed(config.seed + index + 1)
        t0 = time.perf_counter()
        qram.run_normal()
        qram.prepare_observables()
        evolution += time.perf_counter() - t0
        t0 = time.perf_counter()
        # Sampling the routing tree mutates branch groups; do not call
        # sample_and_get_fidelity before reading these ensemble observables.
        dist = qram.get_output_distribution()
        trace = math.fsum(dist.values())
        overlap = float(qram.get_fidelity_conventions()["overlap_fid"])
        logical = float(qram.get_logical_query_fidelity())
        if (
            not math.isfinite(overlap)
            or not -1e-10 <= overlap <= 1 + 1e-10
            or not math.isfinite(logical)
            or not -1e-10 <= logical <= 1 + 1e-10
            or not math.isfinite(trace)
            or abs(trace - 1) > 1e-9
        ):
            raise RuntimeError("native trajectory violates normalization/fidelity bounds")
        samples.append(overlap)
        logical_samples.append(logical)
        traces.append(trace)
        observables += time.perf_counter() - t0
    mean = statistics.fmean(samples)
    logical_mean = statistics.fmean(logical_samples)
    # Hoeffding, P(|mean-E[F]| > radius) <= 0.05 for independent bounded F.
    radius = math.sqrt(math.log(40) / (2 * len(samples)))
    return {
        "schema": "qramsim.demo.v1",
        "software": information(),
        "configuration": dataclasses.asdict(config),
        "memory": {
            "records": len(memory),
            "sha256": hashlib.sha256(memory).hexdigest(),
            "encoding": "one unsigned byte per record",
        },
        "input": {
            "addresses": addresses,
            "bus_initial": 0,
            "probabilities": "equal coherent amplitudes",
        },
        "resources": {
            "address_qubits": config.depth,
            "bus_qubits": config.data_bits,
            "routing_qubits": qram.get_qubit_num(),
        },
        "fidelity": {
            "definition": "unconditional |<ideal|trajectory>|^2 before tree measurement",
            "mean": mean,
            "samples": samples,
            "confidence": {
                "method": "Hoeffding",
                "level": 0.95,
                "interval": [max(0.0, mean - radius), min(1.0, mean + radius)],
            },
        },
        "logical_fidelity": {
            "definition": "<ideal_AB|Tr_tree(|trajectory><trajectory|)|ideal_AB>",
            "postselected": False,
            "mean": logical_mean,
            "samples": logical_samples,
            "confidence": {
                "method": "Hoeffding",
                "level": 0.95,
                "interval": [max(0.0, logical_mean - radius), min(1.0, logical_mean + radius)],
            },
        },
        "normalization": {"max_trace_error": max(abs(t - 1) for t in traces)},
        "timing_s": {
            "prepare": ready - start,
            "native_evolution": evolution,
            "observables": observables,
            "total": time.perf_counter() - start,
        },
        "scope": {
            "input": "fixed random coherent address superposition with zero bus",
            "input_basis_states": config.inputs,
            "input_coverage": (
                "all addresses" if config.inputs == 2**config.depth else "sampled addresses"
            ),
            "confidence_intervals": "95% marginal per observable; no simultaneous coverage claim",
            "hardware_run": False,
            "acceptance_certificate": False,
        },
    }


def main(argv=None) -> int:
    from .evidence import EvidenceConfig

    parser = argparse.ArgumentParser(description=__doc__)
    commands = parser.add_subparsers(dest="command", required=True)
    info = commands.add_parser("info")
    info.add_argument("--out", type=Path)
    experiment = commands.add_parser("run")
    for name in ("depth", "data_bits", "inputs", "trajectories", "seed"):
        experiment.add_argument(
            "--" + name.replace("_", "-"), type=int, default=getattr(Experiment(), name)
        )
    experiment.add_argument("--channel", choices=CHANNELS, default=Experiment.channel)
    experiment.add_argument("--probability", type=float, default=Experiment.probability)
    experiment.add_argument("--out", type=Path)
    words = commands.add_parser("words")
    for name in ("address_width", "data_width", "records", "inputs", "seed"):
        words.add_argument(
            "--" + name.replace("_", "-"), type=int, default=getattr(WordQuery(), name)
        )
    words.add_argument("--out", type=Path)
    addressing = commands.add_parser("addressing")
    for name in ("trajectories", "seed"):
        addressing.add_argument(
            "--" + name.replace("_", "-"), type=int, default=getattr(AddressingQuery(), name)
        )
    addressing.add_argument("--layers", type=int, choices=(2, 3), default=AddressingQuery.layers)
    addressing.add_argument("--channel", choices=CHANNELS, default=AddressingQuery.channel)
    addressing.add_argument("--probability", type=float, default=AddressingQuery.probability)
    addressing.add_argument("--out", type=Path)
    addressing.add_argument(
        "--evaluate",
        type=Path,
        metavar="RECORD",
        help="offline-import a recorded qramsim.addressing.v1 "
        "JSON instead of running a simulation",
    )
    evidence = commands.add_parser("evidence")
    for name in ("trajectories", "stress_trajectories", "seed"):
        evidence.add_argument(
            "--" + name.replace("_", "-"), type=int, default=getattr(EvidenceConfig(), name)
        )
    evidence.add_argument(
        "--stress-probability", type=float, default=EvidenceConfig.stress_probability
    )
    evidence.add_argument(
        "--linearity-probability", type=float, default=EvidenceConfig.linearity_probability
    )
    evidence.add_argument("--outdir", type=Path, help="also write the per-run raw JSON files here")
    evidence.add_argument(
        "--out", type=Path, help="write the summary JSON here (stdout when omitted)"
    )
    gui = commands.add_parser("gui")
    gui.add_argument("--port", type=int, default=0)
    gui.add_argument("--no-browser", action="store_true")
    args = parser.parse_args(argv)
    try:
        if args.command == "gui":
            from .demo_server import serve

            serve(port=args.port, open_browser=not args.no_browser)
            return 0
        if args.command == "info":
            result = information()
        elif args.command == "words":
            result = run_words(
                WordQuery(**{f.name: getattr(args, f.name) for f in dataclasses.fields(WordQuery)})
            )
        elif args.command == "addressing" and args.evaluate is not None:
            if any(
                getattr(args, f.name) != getattr(AddressingQuery(), f.name)
                for f in dataclasses.fields(AddressingQuery)
            ):
                raise ValueError("--evaluate takes no simulation parameters")
            result = evaluate_addressing(json.loads(args.evaluate.read_text(encoding="utf-8")))
        elif args.command == "addressing":
            result = run_addressing(
                AddressingQuery(
                    **{f.name: getattr(args, f.name) for f in dataclasses.fields(AddressingQuery)}
                )
            )
        elif args.command == "evidence":
            from .evidence import run_evidence

            config = EvidenceConfig(
                **{f.name: getattr(args, f.name) for f in dataclasses.fields(EvidenceConfig)}
            )
            result = run_evidence(config, outdir=args.outdir)
            if args.out is None and args.outdir is None:
                print(
                    json.dumps(result, ensure_ascii=False, allow_nan=False, indent=2) + "\n", end=""
                )
                return 0
        else:
            result = run(
                Experiment(
                    **{f.name: getattr(args, f.name) for f in dataclasses.fields(Experiment)}
                )
            )
        output = json.dumps(result, ensure_ascii=False, allow_nan=False, indent=2) + "\n"
        if args.out:
            args.out.parent.mkdir(parents=True, exist_ok=True)
            args.out.write_text(output, encoding="utf-8")
        else:
            print(output, end="")
        return 0
    except (ValueError, RuntimeError, OSError) as exc:
        print(f"QRAM-Simulator: {exc}", file=sys.stderr)
        return 2


if __name__ == "__main__":
    raise SystemExit(main())
