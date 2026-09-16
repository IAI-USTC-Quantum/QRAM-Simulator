"""Execute QECC.Lang RIR JSON documents directly on PySparQ sparse states.

RIR (the *QECC.Lang* intermediate representation produced by the
``pyqecclang`` package) is a typed, register-level IR: programs declare named
registers with kinds (``bits`` / ``uint`` / ``sint`` / ``rational``) and
widths, QRAM resources, and a module graph whose bodies mix gate-level
primitives with structured control nodes (``Call`` / ``Repeat`` /
``Control`` / ``Adjoint``).  This module turns PySparQ into a native RIR
backend: it loads the versioned JSON encoding (schema versions ``0.1``,
``0.2`` and ``0.3``) and interprets it on :class:`pysparq.SparseState`,
*expanding modules at interpretation time* — calls are inlined through
register renaming, ``Repeat`` bodies are replayed, ``Adjoint`` walks the
body backwards with inverted operations, and ``Control`` accumulates
multi-bit conditions — while mapping register-level arithmetic onto native
PySparQ operators (``Add_ConstUInt_InPlace``, ``GlobalPhase_Int``,
``QRAMLoad``) whenever the RIR operand aligns with a whole register.

The interpreter is deliberately independent of ``pyqecclang`` itself: it
consumes only the JSON document, so the two implementations can be
cross-validated against each other and against the OriginIR-ext /
UnifiedQuantum path.

Example:
    >>> import pysparq
    >>> result = pysparq.run_rir(program_json, memory={"values": [1, 2, 3, 0]})
    >>> result.amplitudes[(0, 1)]
    (0.7071067811865476+0j)
"""

from __future__ import annotations

import cmath
import json
import math
from dataclasses import dataclass
from pathlib import Path
from typing import Any, Iterator, Mapping, Sequence

import pysparq._core as _core

__all__ = ["RIRError", "RIRResult", "load_rir", "run_rir", "run_rir_file"]

_SUPPORTED_VERSIONS = {"0.1", "0.2", "0.3"}

_STORAGE_KINDS = {
    "bits": _core.StateStorageType.General,
    "uint": _core.StateStorageType.UnsignedInteger,
    "sint": _core.StateStorageType.SignedInteger,
    "rational": _core.StateStorageType.Rational,
}


class RIRError(ValueError):
    """Raised for malformed RIR documents or execution-limit violations."""


@dataclass(frozen=True)
class RIRResult:
    """Outcome of an RIR execution.

    ``registers`` lists the entry-module registers as ``(name, width)``
    pairs in declaration order; ``amplitudes`` maps each tuple of register
    integer values (same order) to its complex amplitude.
    """

    registers: tuple[tuple[str, int], ...]
    amplitudes: dict[tuple[int, ...], complex]

    def statevector(self, max_qubits: int = 20) -> list[complex]:
        """Dense little-endian statevector over the entry registers."""
        width = sum(w for _, w in self.registers)
        if width > max_qubits:
            raise RIRError(f"dense state conversion exceeds {max_qubits} qubits")
        vector = [0j] * (1 << width)
        for values, amplitude in self.amplitudes.items():
            index, offset = 0, 0
            for (_, w), value in zip(self.registers, values):
                index |= value << offset
                offset += w
            vector[index] = amplitude
        return vector


def load_rir(source: str | Path | Mapping[str, Any]) -> dict[str, Any]:
    """Load and minimally validate an RIR JSON document.

    ``source`` may be a mapping that already holds the decoded document, a
    filesystem path, or a JSON string.  Returns the decoded ``dict``.
    """
    if isinstance(source, Mapping):
        document = dict(source)
    else:
        text = str(source)
        if not text.lstrip().startswith("{"):
            text = Path(text).read_text(encoding="utf-8")
        try:
            document = json.loads(text)
        except json.JSONDecodeError as exc:
            raise RIRError(f"invalid RIR JSON: {exc}") from exc
    if not isinstance(document, dict) or document.get("tag") != "Program":
        raise RIRError("RIR root node must be a Program object")
    version = document.get("version")
    if version not in _SUPPORTED_VERSIONS:
        raise RIRError(f"unsupported RIR schema version: {version!r}")
    modules = document.get("modules")
    if not isinstance(modules, list) or not modules:
        raise RIRError("RIR program must declare at least one module")
    names = [m.get("name") for m in modules if isinstance(m, dict)]
    if len(set(names)) != len(modules) or any(n is None for n in names):
        raise RIRError("RIR module names must be unique")
    if document.get("entry") not in names:
        raise RIRError(f"entry module {document.get('entry')!r} not declared")
    return document


def run_rir_file(
    path: str | Path,
    memory: Mapping[str, Any] | None = None,
    *,
    max_steps: int = 1_000_000,
    max_states: int = 65_536,
) -> RIRResult:
    """Convenience wrapper: load an RIR JSON file and execute it."""
    return run_rir(load_rir(path), memory, max_steps=max_steps, max_states=max_states)


# ---------------------------------------------------------------------------
# Gate matrices (qubit order: the single acted-on bit; row-major 2x2).
# ---------------------------------------------------------------------------


def _gate_matrix(op: str, angle: float | None) -> tuple[tuple[complex, ...], ...]:
    if op == "h":
        a = 1 / math.sqrt(2)
        return ((a, a), (a, -a))
    if op == "x":
        return ((0, 1), (1, 0))
    if op == "y":
        return ((0, -1j), (1j, 0))
    if op == "z":
        return ((1, 0), (0, -1))
    if op in {"s", "t", "phase"}:
        theta = {"s": math.pi / 2, "t": math.pi / 4}.get(op, angle)
        if theta is None:
            raise RIRError(f"gate {op!r} requires an angle")
        return ((1, 0), (0, cmath.exp(1j * theta)))
    if op == "rz":
        return ((cmath.exp(-0.5j * angle), 0), (0, cmath.exp(0.5j * angle)))
    if op in {"rx", "ry"}:
        c, s = math.cos(angle / 2), math.sin(angle / 2)
        if op == "rx":
            return ((c, -1j * s), (-1j * s, c))
        return ((c, -s), (s, c))
    raise RIRError(f"unknown single-qubit gate: {op!r}")


def _dagger(matrix: tuple[tuple[complex, ...], ...]) -> tuple[tuple[complex, ...], ...]:
    return tuple(tuple(matrix[j][i].conjugate() for j in range(2)) for i in range(2))


# ---------------------------------------------------------------------------
# Module-graph expansion.
# ---------------------------------------------------------------------------

Bit = tuple[str, int]  # (native register name, bit index)
Bits = list[Bit]

_EVENT_ENTER = "enter"
_EVENT_EXIT = "exit"
_EVENT_GATE = "gate"
_EVENT_GPHASE = "gphase"
_EVENT_XOR = "xor"
_EVENT_SWAP = "swap"
_EVENT_ADD = "add_const"
_EVENT_LOAD = "load"


def _ref_bits(ref: Mapping[str, Any], mapping: Mapping[str, Bits]) -> Bits:
    bits: Bits = []
    for span in ref["parts"]:
        chunk = mapping[span["register"]]
        bits.extend(chunk[span["start"] : span["start"] + span["width"]])
    return bits


class _Expander:
    """Walks a module graph and yields flat execution events."""

    def __init__(self, document: Mapping[str, Any], max_steps: int):
        self.modules = {m["name"]: m for m in document["modules"]}
        self.max_steps = max_steps
        self._local_counter = 0
        self._count_cache: dict[str, int] = {}

    def _count_body(self, nodes: Sequence[Mapping[str, Any]]) -> int:
        total = 0
        for node in nodes:
            tag = node["tag"]
            if tag in {"Primitive", "Load"}:
                cost = 1
            elif tag == "Call":
                target = node["module"]
                if target not in self._count_cache:
                    body = self.modules[target].get("body")
                    if body is None:
                        raise RIRError(f"module {target!r} has no implementation")
                    self._count_cache[target] = self._count_body(body)
                cost = 1 + self._count_cache[target]
            elif tag == "Repeat":
                cost = int(node["count"]) * self._count_body(node["body"])
            elif tag in {"Control", "Adjoint"}:
                cost = 1 + self._count_body(node["body"])
            else:
                raise RIRError(f"unknown RIR node tag: {tag!r}")
            total += cost
            if total > self.max_steps:
                return total
        return total

    def check_budget(self, entry: str) -> None:
        body = self.modules[entry].get("body")
        if body is None:
            raise RIRError(f"entry module {entry!r} has no implementation")
        if self._count_body(body) > self.max_steps:
            raise RIRError(f"RIR expansion exceeds the step budget ({self.max_steps})")

    def events(
        self,
        module_name: str,
        mapping: dict[str, Bits],
        resources: dict[str, str],
        controls: tuple[tuple[Bits, int], ...] = (),
        inverse: bool = False,
    ) -> Iterator[tuple]:
        module = self.modules[module_name]
        body = module.get("body")
        if body is None:
            raise RIRError(f"module {module_name!r} has no implementation")
        mapping = dict(mapping)
        allocated: list[tuple[str, str, int]] = []
        for register in module.get("locals") or []:
            self._local_counter += 1
            native = f"rir_local_{self._local_counter}"
            width = register["type"]["width"]
            mapping[register["name"]] = [(native, i) for i in range(width)]
            allocated.append((native, register["type"]["kind"], width))
            yield (_EVENT_ENTER, native, register["type"]["kind"], width)
        yield from self._walk(body, mapping, resources, controls, inverse)
        for native, _kind, width in reversed(allocated):
            yield (_EVENT_EXIT, native, width)

    def _walk(
        self,
        nodes: Sequence[Mapping[str, Any]],
        mapping: dict[str, Bits],
        resources: dict[str, str],
        controls: tuple[tuple[Bits, int], ...],
        inverse: bool,
    ) -> Iterator[tuple]:
        for node in reversed(nodes) if inverse else nodes:
            tag = node["tag"]
            if tag == "Call":
                target = self.modules[node["module"]]
                actuals = {
                    formal["name"]: _ref_bits(actual, mapping)
                    for formal, actual in zip(target["registers"], node["arguments"])
                }
                bound = {
                    formal["name"]: resources[actual]
                    for formal, actual in zip(target["resources"], node["resources"])
                }
                yield from self.events(node["module"], actuals, bound, controls, inverse)
            elif tag == "Repeat":
                for _ in range(int(node["count"])):
                    yield from self._walk(node["body"], mapping, resources, controls, inverse)
            elif tag == "Control":
                extra = ((_ref_bits(node["register"], mapping), int(node["value"])),)
                yield from self._walk(
                    node["body"], mapping, resources, controls + extra, inverse
                )
            elif tag == "Adjoint":
                yield from self._walk(node["body"], mapping, resources, controls, not inverse)
            elif tag == "Primitive":
                yield from self._primitive(node, mapping, controls, inverse)
            elif tag == "Load":
                yield (
                    _EVENT_LOAD,
                    resources[node["resource"]],
                    _ref_bits(node["address"], mapping),
                    _ref_bits(node["data"], mapping),
                    controls,
                )
            else:  # pragma: no cover - guarded by _count_body
                raise RIRError(f"unknown RIR node tag: {tag!r}")

    def _primitive(
        self,
        node: Mapping[str, Any],
        mapping: dict[str, Bits],
        controls: tuple[tuple[Bits, int], ...],
        inverse: bool,
    ) -> Iterator[tuple]:
        op = node["op"]
        operands = [_ref_bits(ref, mapping) for ref in node["operands"]]
        if op == "gphase":
            theta = float(node["angle"]) * (-1 if inverse else 1)
            yield (_EVENT_GPHASE, theta, controls)
        elif op in {"xor", "swap"}:
            yield (op, operands[0], operands[1], controls)
        elif op == "add_const":
            value = int(node["value"]) * (-1 if inverse else 1)
            yield (_EVENT_ADD, operands[0], value, controls)
        else:
            matrix = _gate_matrix(op, node.get("angle"))
            if inverse:
                matrix = _dagger(matrix)
            yield (_EVENT_GATE, matrix, operands[0], controls)


# ---------------------------------------------------------------------------
# Sparse-state executor.
# ---------------------------------------------------------------------------


def _check_memory(
    resources: Sequence[Mapping[str, Any]], memory: Mapping[str, Any] | None
) -> dict[str, dict[int, int]]:
    memory = {} if memory is None else memory
    if not isinstance(memory, Mapping):
        raise RIRError("QRAM data must be provided as a mapping keyed by resource name")
    specs = {r["name"]: r["type"] for r in resources}
    if set(memory) != set(specs):
        raise RIRError("memory must provide exactly one entry per declared QRAM resource")
    result: dict[str, dict[int, int]] = {}
    for name, spec in specs.items():
        raw = memory[name]
        items = raw.items() if isinstance(raw, Mapping) else enumerate(raw)
        cells: dict[int, int] = {}
        for address, value in items:
            if not isinstance(address, int) or not 0 <= address < 1 << spec["address_width"]:
                raise RIRError(f"QRAM {name!r}: address out of range: {address!r}")
            if not isinstance(value, int) or not 0 <= value < 1 << spec["data_width"]:
                raise RIRError(f"QRAM {name!r}: word out of range: {value!r}")
            if value:
                cells[address] = value
        result[name] = cells
    return result


class _Runner:
    def __init__(self, document: Mapping[str, Any], memory: Mapping[str, Any] | None, max_states: int):
        modules = {m["name"]: m for m in document["modules"]}
        self.entry = modules[document["entry"]]
        self.max_states = max_states
        self.memories = _check_memory(self.entry["resources"], memory)
        for resource in self.entry["resources"]:
            if resource["type"]["address_width"] > 20:
                raise RIRError("refusing to materialize a QRAM with more than 2^20 cells")
        self.state = _core.SparseState()
        self.qrams: dict[str, Any] = {}
        self.widths: dict[str, int] = {}

    def _add_register(self, native: str, kind: str, width: int) -> None:
        _core.AddRegister(native, _STORAGE_KINDS[kind], width)(self.state)
        self.widths[native] = width

    def setup(self) -> tuple[dict[str, Bits], dict[str, str]]:
        mapping: dict[str, Bits] = {}
        for i, register in enumerate(self.entry["registers"]):
            width = register["type"]["width"]
            if not width:
                mapping[register["name"]] = []
                continue
            native = f"rir_root_{i}"
            self._add_register(native, register["type"]["kind"], width)
            mapping[register["name"]] = [(native, bit) for bit in range(width)]
        resources = {}
        for resource in self.entry["resources"]:
            spec = resource["type"]
            data = [0] * (1 << spec["address_width"])
            for address, value in self.memories[resource["name"]].items():
                data[address] = value
            qram = _core.QRAMCircuit_qutrit(spec["address_width"], spec["data_width"], data)
            self.qrams[resource["name"]] = qram
            resources[resource["name"]] = resource["name"]
        return mapping, resources

    # -- low-level application helpers -------------------------------------

    def _apply(self, operator: Any, controls: Sequence[Bit] = ()) -> None:
        if controls:
            operator.conditioned_by_bit(list(controls))
        operator(self.state)
        if len(self.state.basis_states) > self.max_states:
            raise RIRError(f"execution exceeds the sparse-state budget ({self.max_states})")

    def _with_conditions(self, controls: tuple[tuple[Bits, int], ...], body) -> None:
        flat: list[Bit] = []
        zeros: list[Bit] = []
        for bits, expected in controls:
            for index, pair in enumerate(bits):
                flat.append(pair)
                if not (expected >> index) & 1:
                    zeros.append(pair)
        for pair in zeros:
            _core.Xgate_Bool(*pair)(self.state)
        try:
            body(tuple(flat))
        finally:
            for pair in reversed(zeros):
                _core.Xgate_Bool(*pair)(self.state)

    # -- events --------------------------------------------------------------

    def run_event(self, event: tuple) -> None:
        kind = event[0]
        if kind == _EVENT_ENTER:
            _, native, reg_kind, width = event
            if width:
                self._add_register(native, reg_kind, width)
            return
        if kind == _EVENT_EXIT:
            _, native, width = event
            if width:
                rid = _core.System.get_id(native)
                mask = (1 << width) - 1
                if any(int(basis.get(rid).value) & mask for basis in self.state.basis_states):
                    raise RIRError(f"local register not uncomputed: {native}")
                _core.RemoveRegister(native)(self.state)
            return
        if kind == _EVENT_GPHASE:
            _, theta, controls = event
            self._gphase(theta, controls)
            return
        if kind == _EVENT_GATE:
            _, matrix, bits, controls = event
            flat = [v for row in matrix for v in row]
            self._with_conditions(
                controls, lambda c: [self._apply(_core.Rot_Bool(*bit, flat), c) for bit in bits]
            )
            return
        if kind in {_EVENT_XOR, _EVENT_SWAP}:
            _, source, target, controls = event
            self._bitwise(kind, source, target, controls)
            return
        if kind == _EVENT_ADD:
            _, bits, value, controls = event
            self._add_const(bits, value, controls)
            return
        if kind == _EVENT_LOAD:
            _, qram_name, address, data, controls = event
            self._load(qram_name, address, data, controls)
            return
        raise RIRError(f"unknown execution event: {kind!r}")  # pragma: no cover

    def _gphase(self, theta: float, controls: tuple[tuple[Bits, int], ...]) -> None:
        if not controls:
            self._apply(_core.GlobalPhase_Int(cmath.exp(1j * theta)))
            return
        flat: list[Bit] = []
        zeros: list[Bit] = []
        for bits, expected in controls:
            for index, pair in enumerate(bits):
                flat.append(pair)
                if not (expected >> index) & 1:
                    zeros.append(pair)
        target = flat[-1]
        rest = tuple(flat[:-1])
        for pair in zeros:
            _core.Xgate_Bool(*pair)(self.state)
        try:
            self._apply(_core.Phase_Bool(*target, theta), rest)
        finally:
            for pair in reversed(zeros):
                _core.Xgate_Bool(*pair)(self.state)

    def _bitwise(
        self, op: str, source: Bits, target: Bits, controls: tuple[tuple[Bits, int], ...]
    ) -> None:
        if len(source) != len(target):
            raise RIRError(f"{op} operands must have equal width")

        def body(c: tuple[Bit, ...]) -> None:
            for a, b in zip(source, target):
                self._apply(_core.Xgate_Bool(*b), c + (a,))
                if op == _EVENT_SWAP:
                    self._apply(_core.Xgate_Bool(*a), c + (b,))
                    self._apply(_core.Xgate_Bool(*b), c + (a,))

        self._with_conditions(controls, body)

    def _add_const(
        self, bits: Bits, value: int, controls: tuple[tuple[Bits, int], ...]
    ) -> None:
        if not bits:
            return
        width = len(bits)
        value %= 1 << width
        names = {name for name, _ in bits}
        # 仅当操作数完整覆盖寄存器（起点 0 且宽度等于寄存器声明宽度）时
        # 才能用原生整数加法；寄存器切片必须走逐位回退以获得视图宽度的回绕语义。
        native_full = len(names) == 1 and bits == [
            (bits[0][0], i) for i in range(self.widths.get(bits[0][0], -1))
        ]
        if native_full:
            self._with_conditions(
                controls,
                lambda c: self._apply(_core.Add_ConstUInt_InPlace(bits[0][0], value), c),
            )
            return

        def body(c: tuple[Bit, ...]) -> None:
            for offset in range(width):
                if (value >> offset) & 1:
                    for i in reversed(range(offset + 1, width)):
                        self._apply(_core.Xgate_Bool(*bits[i]), c + tuple(bits[offset:i]))
                    self._apply(_core.Xgate_Bool(*bits[offset]), c)

        self._with_conditions(controls, body)

    def _load(
        self,
        qram_name: str,
        address: Bits,
        data: Bits,
        controls: tuple[tuple[Bits, int], ...],
    ) -> None:
        addr_tmp, data_tmp = "rir_tmp_addr", "rir_tmp_data"
        self._add_register(addr_tmp, "bits", len(address))
        self._add_register(data_tmp, "bits", len(data))
        try:

            def body(c: tuple[Bit, ...]) -> None:
                for i, pair in enumerate(address):
                    self._apply(_core.Xgate_Bool(addr_tmp, i), c + (pair,))
                query = _core.QRAMLoad(self.qrams[qram_name], addr_tmp, data_tmp)
                query(self.state)
                for i, pair in enumerate(data):
                    self._apply(_core.Xgate_Bool(*pair), c + ((data_tmp, i),))
                query(self.state)
                for i, pair in reversed(list(enumerate(address))):
                    self._apply(_core.Xgate_Bool(addr_tmp, i), c + (pair,))

            self._with_conditions(controls, body)
        finally:
            _core.RemoveRegister(data_tmp)(self.state)
            _core.RemoveRegister(addr_tmp)(self.state)

    def result(self, mapping: dict[str, Bits]) -> RIRResult:
        registers = tuple(
            (register["name"], register["type"]["width"]) for register in self.entry["registers"]
        )
        amplitudes: dict[tuple[int, ...], complex] = {}
        ids = {
            register["name"]: _core.System.get_id(bits[0][0])
            for register in self.entry["registers"]
            for bits in [mapping[register["name"]]]
            if bits
        }
        widths = {register["name"]: register["type"]["width"] for register in self.entry["registers"]}
        for basis in self.state.basis_states:
            key = tuple(
                (int(basis.get(ids[name]).value) & ((1 << widths[name]) - 1)) if widths[name] else 0
                for name, _ in registers
            )
            amplitudes[key] = amplitudes.get(key, 0j) + complex(basis.amplitude)
        return RIRResult(
            registers, {k: v for k, v in amplitudes.items() if abs(v) > 1e-15}
        )


def run_rir(
    document: str | Path | Mapping[str, Any],
    memory: Mapping[str, Any] | None = None,
    *,
    max_steps: int = 1_000_000,
    max_states: int = 65_536,
) -> RIRResult:
    """Execute an RIR document on a fresh :class:`pysparq.SparseState`.

    Args:
        document: RIR JSON as a decoded mapping, JSON string, or file path.
        memory: QRAM contents keyed by resource name; each value is a word
            sequence or a sparse ``{address: word}`` mapping.
        max_steps: expansion budget over the module graph (calls, repeats and
            controlled regions); exceeded budgets raise :class:`RIRError`.
        max_states: sparse-state budget (number of basis states); exceeded
            budgets raise :class:`RIRError`.

    Returns:
        :class:`RIRResult` with per-register integer amplitudes.
    """
    document = load_rir(document)
    if _core.System.get_activated_register_size():
        raise RIRError("PySparQ global register table is not empty; finish the other run first")
    expander = _Expander(document, max_steps)
    expander.check_budget(document["entry"])
    runner = _Runner(document, memory, max_states)
    try:
        mapping, resources = runner.setup()
        for event in expander.events(document["entry"], mapping, resources):
            runner.run_event(event)
        return runner.result(mapping)
    finally:
        _core.System.clear()
