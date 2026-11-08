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
        limits = {"depth": (1, 20), "data_bits": (1, 8), "inputs": (1, 1024),
                  "trajectories": (1, 4096), "seed": (0, 2**32-1)}
        for name, (lo, hi) in limits.items():
            value = getattr(self, name)
            if type(value) is not int or not lo <= value <= hi:
                raise ValueError(f"{name} must be an integer in [{lo}, {hi}]")
        if self.inputs > 2**self.depth:
            raise ValueError("inputs exceeds the number of addresses")
        if self.channel not in CHANNELS:
            raise ValueError("unknown noise channel")
        if (type(self.probability) not in (int, float) or
                not math.isfinite(self.probability) or not 0 <= self.probability < 1):
            raise ValueError("probability must be finite and in [0,1)")
        if self.channel == "none" and self.probability != 0:
            raise ValueError("the noiseless channel requires probability=0")


def information() -> dict:
    import qram_simulator as qs
    return {"software": "QRAM-Simulator", "version": qs.__version__,
            "python": platform.python_version(), "platform": platform.platform(),
            "backend": "native CPU sparse branches", "gpu_execution": False,
            "damping_convention": qs.QUBIT_DAMPING_CONVENTION,
            "channels": list(CHANNELS), "demo_architecture": "qubit"}


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
    noise = {} if config.channel == "none" else {
        getattr(qs.OperationType, config.channel): config.probability}
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
        if (not math.isfinite(overlap) or not -1e-10 <= overlap <= 1+1e-10 or
                not math.isfinite(logical) or not -1e-10 <= logical <= 1+1e-10 or
                not math.isfinite(trace) or abs(trace-1) > 1e-9):
            raise RuntimeError("native trajectory violates normalization/fidelity bounds")
        samples.append(overlap)
        logical_samples.append(logical)
        traces.append(trace)
        observables += time.perf_counter()-t0
    mean = statistics.fmean(samples)
    logical_mean = statistics.fmean(logical_samples)
    # Hoeffding, P(|mean-E[F]| > radius) <= 0.05 for independent bounded F.
    radius = math.sqrt(math.log(40)/(2*len(samples)))
    return {"schema": "qramsim.demo.v1", "software": information(),
            "configuration": dataclasses.asdict(config),
            "memory": {"records": len(memory), "sha256": hashlib.sha256(memory).hexdigest(),
                       "encoding": "one unsigned byte per record"},
            "input": {"addresses": addresses, "bus_initial": 0,
                      "probabilities": "equal coherent amplitudes"},
            "resources": {"address_qubits": config.depth, "bus_qubits": config.data_bits,
                          "routing_qubits": qram.get_qubit_num()},
            "fidelity": {"definition": "unconditional |<ideal|trajectory>|^2 before tree measurement",
                         "mean": mean, "samples": samples,
                         "confidence": {"method": "Hoeffding", "level": 0.95,
                                        "interval": [max(0.0, mean-radius), min(1.0, mean+radius)]}},
            "logical_fidelity": {"definition": "<ideal_AB|Tr_tree(|trajectory><trajectory|)|ideal_AB>",
                                 "postselected": False, "mean": logical_mean,
                                 "samples": logical_samples,
                                 "confidence": {"method": "Hoeffding", "level": 0.95,
                                     "interval": [max(0.0, logical_mean-radius), min(1.0, logical_mean+radius)]}},
            "normalization": {"max_trace_error": max(abs(t-1) for t in traces)},
            "timing_s": {"prepare": ready-start, "native_evolution": evolution,
                         "observables": observables,
                         "total": time.perf_counter()-start},
            "scope": {"input": "fixed random coherent address superposition with zero bus",
                      "input_basis_states": config.inputs,
                      "input_coverage": ("all addresses" if config.inputs == 2**config.depth
                                         else "sampled addresses"),
                      "confidence_intervals": "95% marginal per observable; no simultaneous coverage claim",
                      "hardware_run": False, "acceptance_certificate": False}}


def main(argv=None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    commands = parser.add_subparsers(dest="command", required=True)
    commands.add_parser("info")
    experiment = commands.add_parser("run")
    for name in ("depth", "data_bits", "inputs", "trajectories", "seed"):
        experiment.add_argument("--"+name.replace("_", "-"), type=int,
                                default=getattr(Experiment(), name))
    experiment.add_argument("--channel", choices=CHANNELS, default=Experiment.channel)
    experiment.add_argument("--probability", type=float, default=Experiment.probability)
    experiment.add_argument("--out", type=Path)
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
        else:
            result = run(Experiment(**{f.name: getattr(args, f.name)
                                      for f in dataclasses.fields(Experiment)}))
        output = json.dumps(result, ensure_ascii=False, allow_nan=False, indent=2)+"\n"
        if args.command == "run" and args.out:
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
