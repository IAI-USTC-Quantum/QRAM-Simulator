"""Distribution metrics shared by both sides of the baseline comparison.

The QRAM-Simulator side returns ``{"addr:bus": prob}`` dictionaries (from
``QRAMCircuitQubit.get_output_distribution``) and the circuit side returns the
same key convention (``CircuitQRAMQubit.marginal_distribution``), so all
metrics below are convention-identical on the two sides:

- ``classical_fidelity``: (sum sqrt(p q))^2 over the joint (addr, bus) keys;
- ``tvd``: total variation distance of the same;
- renormalization is applied per side by its own total (survival / trace)
  before comparison whenever either side is sub-normalized (Damping).
"""

from __future__ import annotations

import math
from typing import Dict, Iterable, Mapping

__all__ = ["classical_fidelity", "tvd", "dist_total", "normalize", "mean_of_dists"]


def _keys(p: Mapping[str, float], q: Mapping[str, float]):
    return set(p) | set(q)


def classical_fidelity(p: Mapping[str, float], q: Mapping[str, float]) -> float:
    """Classical distribution fidelity (sum of sqrt(p*q))^2."""
    return float(sum(math.sqrt(max(p.get(k, 0.0) * q.get(k, 0.0), 0.0))
                     for k in _keys(p, q)) ** 2)


def tvd(p: Mapping[str, float], q: Mapping[str, float]) -> float:
    """Total variation distance 0.5 * sum |p - q|."""
    return 0.5 * float(sum(abs(p.get(k, 0.0) - q.get(k, 0.0)) for k in _keys(p, q)))


def dist_total(p: Mapping[str, float]) -> float:
    """Total mass (survival probability of a sub-normalized distribution)."""
    return float(sum(p.values()))


def normalize(p: Mapping[str, float]) -> Dict[str, float]:
    """Return p divided by its total mass (identity when already normalized)."""
    total = dist_total(p)
    if total <= 0.0:
        return dict(p)
    return {k: v / total for k, v in p.items()}


def mean_of_dists(dists: Iterable[Mapping[str, float]]) -> Dict[str, float]:
    """Element-wise mean over a collection of distributions (missing keys = 0)."""
    acc: Dict[str, float] = {}
    count = 0
    for d in dists:
        count += 1
        for k, v in d.items():
            acc[k] = acc.get(k, 0.0) + v
    if count == 0:
        return {}
    return {k: v / count for k, v in acc.items()}
