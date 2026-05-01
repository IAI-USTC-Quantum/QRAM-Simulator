"""Shared helpers imported by PySparQ's algorithm integration tests."""

from __future__ import annotations

from collections import defaultdict

import pysparq as ps


def state_to_amplitude_dict(
    state: ps.SparseState,
    register: str,
) -> dict[int, complex]:
    """Aggregate amplitudes by the selected register value."""
    reg_id = ps.System.get_id(register)
    amplitudes: defaultdict[int, complex] = defaultdict(complex)
    for basis in state.basis_states:
        amplitudes[int(basis.get(reg_id).value)] += basis.amplitude
    return dict(amplitudes)
