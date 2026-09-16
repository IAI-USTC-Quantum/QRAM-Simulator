"""Tests for :mod:`pysparq.rir` — native RIR execution on PySparQ."""

from __future__ import annotations

import cmath
import math

import pytest

import pysparq as ps


def ref(register, start, width, kind="uint"):
    return {
        "tag": "Ref",
        "parts": [{"tag": "Span", "register": register, "start": start, "width": width}],
        "type": {"tag": "RegType", "kind": kind, "width": width},
    }


def whole(register, width, kind="uint"):
    return ref(register, 0, width, kind)


def prim(op, operands, angle=None, value=None):
    return {"tag": "Primitive", "op": op, "operands": operands, "angle": angle, "value": value}


def register(name, width, kind="uint"):
    return {"tag": "Register", "name": name, "type": {"tag": "RegType", "kind": kind, "width": width}}


def module(name, registers, body, locals_=None, resources=None):
    return {
        "tag": "Module",
        "name": name,
        "registers": registers,
        "locals": locals_ or [],
        "resources": resources or [],
        "body": body,
        "attributes": [],
    }


def program(entry, modules):
    return {"tag": "Program", "entry": entry, "modules": modules, "version": "0.3"}


def call(module_name, arguments, resources=()):
    return {
        "tag": "Call",
        "module": module_name,
        "arguments": arguments,
        "resources": list(resources),
    }


class TestLoadValidation:
    def test_rejects_bad_version(self):
        doc = program("main", [module("main", [], [])])
        doc["version"] = "9.9"
        with pytest.raises(ps.RIRError, match="version"):
            ps.run_rir(doc)

    def test_rejects_unknown_entry(self):
        doc = program("nope", [module("main", [], [])])
        with pytest.raises(ps.RIRError, match="entry"):
            ps.run_rir(doc)

    def test_rejects_non_program_root(self):
        with pytest.raises(ps.RIRError, match="Program"):
            ps.run_rir({"tag": "Module"})

    def test_open_module_body_raises(self):
        doc = program("main", [module("main", [register("q", 1)], None)])
        with pytest.raises(ps.RIRError, match="implementation"):
            ps.run_rir(doc)


class TestGateLevel:
    def test_bell_state(self):
        body = [
            prim("h", [whole("a", 1, "bits")]),
            {"tag": "Control", "register": whole("a", 1, "bits"), "value": 1,
             "body": [prim("x", [whole("b", 1, "bits")])]},
        ]
        doc = program("main", [module("main", [register("a", 1, "bits"), register("b", 1, "bits")], body)])
        result = ps.run_rir(doc)
        a = 1 / math.sqrt(2)
        assert result.amplitudes == {(0, 0): pytest.approx(a), (1, 1): pytest.approx(a)}

    def test_control_on_zero(self):
        body = [
            {"tag": "Control", "register": whole("a", 1, "bits"), "value": 0,
             "body": [prim("x", [whole("b", 1, "bits")])]},
        ]
        doc = program("main", [module("main", [register("a", 1, "bits"), register("b", 1, "bits")], body)])
        result = ps.run_rir(doc)
        assert result.amplitudes == {(0, 1): pytest.approx(1.0)}

    def test_global_phase_native(self):
        body = [prim("gphase", [], angle=math.pi)]
        doc = program("main", [module("main", [register("q", 1, "bits")], body)])
        result = ps.run_rir(doc)
        assert result.amplitudes == {(0,): pytest.approx(-1.0)}

    def test_controlled_gphase(self):
        body = [
            prim("x", [whole("q", 1, "bits")]),
            {"tag": "Control", "register": whole("q", 1, "bits"), "value": 1,
             "body": [prim("gphase", [], angle=math.pi / 2)]},
        ]
        doc = program("main", [module("main", [register("q", 1, "bits")], body)])
        result = ps.run_rir(doc)
        assert result.amplitudes == {(1,): pytest.approx(1j)}

    def test_rotation_and_adjoint(self):
        body = [
            prim("rx", [whole("q", 1, "bits")], angle=0.7),
            {"tag": "Adjoint", "body": [prim("rx", [whole("q", 1, "bits")], angle=0.7)]},
        ]
        doc = program("main", [module("main", [register("q", 1, "bits")], body)])
        result = ps.run_rir(doc)
        assert result.amplitudes == {(0,): pytest.approx(1.0)}

    def test_xor_and_swap_views(self):
        body = [
            prim("x", [whole("a", 2)]),
            prim("xor", [whole("a", 2), whole("b", 2)]),
            prim("swap", [whole("a", 2), whole("c", 2)]),
        ]
        doc = program(
            "main",
            [module("main", [register("a", 2), register("b", 2), register("c", 2)], body)],
        )
        result = ps.run_rir(doc)
        assert result.amplitudes == {(0, 3, 3): pytest.approx(1.0)}


class TestModuleExpansion:
    def test_call_repeat_adjoint_arithmetic(self):
        inc = module("inc", [register("x", 4)], [prim("add_const", [whole("x", 4)], value=1)])
        main = module(
            "main",
            [register("x", 4)],
            [
                {"tag": "Repeat", "count": 3, "body": [call("inc", [whole("x", 4)])]},
                {"tag": "Adjoint", "body": [call("inc", [whole("x", 4)])]},
            ],
        )
        result = ps.run_rir(program("main", [main, inc]))
        assert result.amplitudes == {(2,): pytest.approx(1.0)}

    def test_nested_call_register_views(self):
        # callee adds 1 to the low half it is handed
        callee = module("bump", [register("lo", 2)], [prim("add_const", [whole("lo", 2)], value=1)])
        main = module(
            "main",
            [register("x", 4)],
            [prim("x", [ref("x", 2, 2)]), call("bump", [ref("x", 0, 2)])],
        )
        result = ps.run_rir(program("main", [main, callee]))
        # x = 0b1100 initially, callee adds 1 to low two bits -> 0b1101
        assert result.amplitudes == {(13,): pytest.approx(1.0)}

    def test_repeat_budget(self):
        main = module(
            "main",
            [register("x", 4)],
            [{"tag": "Repeat", "count": 10**9, "body": [prim("x", [whole("x", 4)])]}],
        )
        with pytest.raises(ps.RIRError, match="budget"):
            ps.run_rir(program("main", [main]), max_steps=1000)

    def test_locals_must_uncompute(self):
        work = register("w", 1, "bits")
        clean = module(
            "main",
            [register("out", 1, "bits")],
            [
                prim("x", [whole("w", 1, "bits")]),
                {"tag": "Control", "register": whole("w", 1, "bits"), "value": 1,
                 "body": [prim("x", [whole("out", 1, "bits")])]},
                prim("x", [whole("w", 1, "bits")]),
            ],
            locals_=[work],
        )
        result = ps.run_rir(program("main", [clean]))
        assert result.amplitudes == {(1,): pytest.approx(1.0)}

        dirty = module(
            "main",
            [register("out", 1, "bits")],
            [prim("x", [whole("w", 1, "bits")])],
            locals_=[work],
        )
        with pytest.raises(ps.RIRError, match="uncomputed"):
            ps.run_rir(program("main", [dirty]))


class TestArithmetic:
    def test_add_const_full_register_uses_native(self):
        body = [prim("add_const", [whole("x", 5)], value=63)]
        doc = program("main", [module("main", [register("x", 5)], body)])
        result = ps.run_rir(doc)
        assert result.amplitudes == {(63 % 32,): pytest.approx(1.0)}

    def test_add_const_partial_view_falls_back(self):
        body = [
            prim("x", [ref("x", 0, 1)]),
            prim("x", [ref("x", 2, 1)]),
            prim("add_const", [ref("x", 0, 2)], value=1),
        ]
        doc = program("main", [module("main", [register("x", 4)], body)])
        result = ps.run_rir(doc)
        # x = 0b0101; low two bits 01 + 1 = 10 -> 0b0110
        assert result.amplitudes == {(6,): pytest.approx(1.0)}

    def test_add_const_low_slice_wraps_at_view_width(self):
        body = [
            prim("x", [ref("x", 0, 1)]),
            prim("x", [ref("x", 1, 1)]),
            prim("add_const", [ref("x", 0, 2)], value=1),
        ]
        doc = program("main", [module("main", [register("x", 4)], body)])
        result = ps.run_rir(doc)
        # x = 0b0011；视图值 3 + 1 在 2 bit 视图内回绕为 0 -> x = 0
        assert result.amplitudes == {(0,): pytest.approx(1.0)}

    def test_add_const_high_slice(self):
        body = [prim("add_const", [ref("x", 2, 2)], value=1)]
        doc = program("main", [module("main", [register("x", 4)], body)])
        result = ps.run_rir(doc)
        assert result.amplitudes == {(4,): pytest.approx(1.0)}

        wrap = module(
            "main",
            [register("x", 4)],
            [
                prim("x", [ref("x", 2, 1)]),
                prim("x", [ref("x", 3, 1)]),
                prim("add_const", [ref("x", 2, 2)], value=1),
            ],
        )
        result = ps.run_rir(program("main", [wrap]))
        # 高位视图 0b11 + 1 回绕为 0 -> x = 0
        assert result.amplitudes == {(0,): pytest.approx(1.0)}

    def test_controlled_add_const(self):
        body = [
            prim("x", [whole("flag", 1, "bits")]),
            {"tag": "Control", "register": whole("flag", 1, "bits"), "value": 1,
             "body": [prim("add_const", [whole("x", 3)], value=5)]},
        ]
        doc = program("main", [module("main", [register("flag", 1, "bits"), register("x", 3)], body)])
        result = ps.run_rir(doc)
        assert result.amplitudes == {(1, 5): pytest.approx(1.0)}


class TestQRAM:
    def _doc(self):
        values = {
            "tag": "Resource",
            "name": "values",
            "type": {"tag": "QRAM", "address_width": 2, "data_width": 3},
        }
        body = [
            prim("h", [whole("address", 2)]),
            {"tag": "Load", "resource": "values",
             "address": whole("address", 2), "data": whole("data", 3)},
        ]
        main = module("main", [register("address", 2), register("data", 3)], body, resources=[values])
        return program("main", [main])

    def test_load_over_superposition(self):
        result = ps.run_rir(self._doc(), memory={"values": [1, 2, 4, 7]})
        expected = {
            (address, word): pytest.approx(0.5)
            for address, word in enumerate([1, 2, 4, 7])
        }
        assert result.amplitudes == expected

    def test_sparse_memory_and_zero_cells(self):
        result = ps.run_rir(self._doc(), memory={"values": {2: 5}})
        assert result.amplitudes[(2, 5)] == pytest.approx(0.5)
        assert result.amplitudes[(0, 0)] == pytest.approx(0.5)

    def test_memory_must_match_resources(self):
        with pytest.raises(ps.RIRError, match="QRAM"):
            ps.run_rir(self._doc(), memory={})
        with pytest.raises(ps.RIRError, match="out of range"):
            ps.run_rir(self._doc(), memory={"values": [8, 0, 0, 0]})

    def test_statevector_layout(self):
        result = ps.run_rir(self._doc(), memory={"values": [1, 2, 4, 7]})
        vector = result.statevector()
        assert vector[(2 << 2) | 1] == pytest.approx(0.5)  # address=1, data=2
        assert vector[(7 << 2) | 3] == pytest.approx(0.5)


class TestFileInterface:
    def test_run_rir_file(self, tmp_path):
        import json

        doc = program("main", [module("main", [register("q", 1, "bits")], [prim("h", [whole("q", 1, "bits")])])])
        path = tmp_path / "bell.rir.json"
        path.write_text(json.dumps(doc))
        result = ps.run_rir_file(path)
        a = 1 / math.sqrt(2)
        assert result.amplitudes == {(0,): pytest.approx(a), (1,): pytest.approx(a)}

    def test_load_rir_accepts_json_string(self):
        import json

        doc = program("main", [module("main", [], [])])
        loaded = ps.load_rir(json.dumps(doc))
        assert loaded["entry"] == "main"
