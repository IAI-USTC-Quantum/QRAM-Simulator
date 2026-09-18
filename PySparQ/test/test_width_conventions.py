"""Width & truncation convention coverage (docs/operators.md《宽度与截断约定》).

Every new integer-core / flag operator plus the migrated existing operators
is run through :func:`pysparq.conformance.width_matrix_case`: independent
mathematical models in pure Python (big-int, full precision) checked against
basis-exhaustive / sampled forward maps, nonzero-output-start collision
freedom, both dagger orders, superposition linearity on small widths, and
the control matrix — across boundary width combinations including the
63/64-bit shift-UB boundaries.
"""

from __future__ import annotations

import math

import pytest

import pysparq as ps
from pysparq.conformance import (
    RegisterSpec,
    make_basis_state,
    read_registers,
    setup_registers,
    sign_extend,
    two_complement_decode,
    width_matrix_case,
)

UINT = ps.UnsignedInteger
SINT = ps.SignedInteger
BOOL = ps.Boolean

# 宽度组合:小/混合/边界(63、64 为移位 UB 修复边界)
COMBOS_2IN = [(1, 1, 1), (2, 3, 2), (3, 5, 4), (4, 4, 4), (5, 8, 3), (8, 8, 8), (63, 2, 8), (64, 64, 64)]
COMBOS_1IN = [(1, 1), (2, 3), (3, 5), (5, 8), (8, 8), (63, 8), (64, 64)]


def _uint(name: str, width: int) -> RegisterSpec:
    return RegisterSpec(name, UINT, width)


def _sint(name: str, width: int) -> RegisterSpec:
    return RegisterSpec(name, SINT, width)


def _bool(name: str) -> RegisterSpec:
    return RegisterSpec(name, BOOL, 1)


def _controls(names_values: dict[str, int], inactive: dict[str, int] | None = None):
    specs = [_bool(n) for n in names_values]
    inactive = inactive or {n: 0 for n in names_values}
    return specs, dict(names_values), inactive


# ---------------------------------------------------------------------------
# 整数核
# ---------------------------------------------------------------------------


def test_sub_uint_uint_width_matrix():
    width_matrix_case(
        label="Sub_UInt_UInt",
        specs_factory=lambda c: [_uint("lhs", c[0]), _uint("rhs", c[1]), _uint("res", c[2])],
        make_op=lambda w: ps.Sub_UInt_UInt("lhs", "rhs", "res"),
        model=lambda v, w: {"res": (v["lhs"] - v["rhs"]) % (1 << w["res"])},
        input_names=["lhs", "rhs"],
        output_names=["res"],
        width_combos=COMBOS_2IN,
        control_case=((3, 5, 4), *_controls({"c": 1})),
    )


def test_neg_uint_width_matrix():
    width_matrix_case(
        label="Neg_UInt",
        specs_factory=lambda c: [_uint("reg", c[0]), _uint("res", c[1])],
        make_op=lambda w: ps.Neg_UInt("reg", "res"),
        model=lambda v, w: {"res": (0 - v["reg"]) % (1 << w["res"])},
        input_names=["reg"],
        output_names=["res"],
        width_combos=COMBOS_1IN,
        control_case=((3, 5), *_controls({"c": 1})),
    )


def test_abs_sint_width_matrix():
    width_matrix_case(
        label="Abs_SInt",
        specs_factory=lambda c: [_sint("reg", c[0]), _uint("res", c[1])],
        make_op=lambda w: ps.Abs_SInt("reg", "res"),
        model=lambda v, w: {"res": abs(sign_extend(v["reg"], w["reg"])) % (1 << w["res"])},
        input_names=["reg"],
        output_names=["res"],
        width_combos=COMBOS_1IN,
        control_case=((3, 5), *_controls({"c": 1})),
    )


def test_mul_uint_uint_width_matrix():
    width_matrix_case(
        label="Mul_UInt_UInt",
        specs_factory=lambda c: [_uint("lhs", c[0]), _uint("rhs", c[1]), _uint("res", c[2])],
        make_op=lambda w: ps.Mul_UInt_UInt("lhs", "rhs", "res"),
        model=lambda v, w: {"res": (v["lhs"] * v["rhs"]) % (1 << w["res"])},
        input_names=["lhs", "rhs"],
        output_names=["res"],
        width_combos=COMBOS_2IN,
        control_case=((3, 5, 4), *_controls({"c": 1})),
    )


def test_div_uint_uint_width_matrix():
    width_matrix_case(
        label="Div_UInt_UInt",
        specs_factory=lambda c: [_uint("lhs", c[0]), _uint("rhs", c[1]), _uint("res", c[2])],
        make_op=lambda w: ps.Div_UInt_UInt("lhs", "rhs", "res"),
        model=lambda v, w: {"res": ((0 if v["rhs"] == 0 else v["lhs"] // v["rhs"])) % (1 << w["res"])},
        input_names=["lhs", "rhs"],
        output_names=["res"],
        width_combos=COMBOS_2IN,
        control_case=((3, 5, 4), *_controls({"c": 1})),
    )


def test_sqrt_uint_width_matrix():
    width_matrix_case(
        label="Sqrt_UInt",
        specs_factory=lambda c: [_uint("reg", c[0]), _uint("res", c[1])],
        make_op=lambda w: ps.Sqrt_UInt("reg", "res"),
        model=lambda v, w: {"res": math.isqrt(v["reg"]) % (1 << w["res"])},
        input_names=["reg"],
        output_names=["res"],
        width_combos=COMBOS_1IN,
        control_case=((3, 5), *_controls({"c": 1})),
    )


def test_select_bool_uint_uint_width_matrix():
    # combo = (cond_w 恒为 1, lhs_w, rhs_w, res_w)
    combos = [(1, a, b, c) for a, b, c in COMBOS_2IN]

    def specs(c):
        return [_bool("cond"), _uint("lhs", c[1]), _uint("rhs", c[2]), _uint("res", c[3])]

    width_matrix_case(
        label="Select_Bool_UInt_UInt",
        specs_factory=specs,
        make_op=lambda w: ps.Select_Bool_UInt_UInt("cond", "lhs", "rhs", "res"),
        model=lambda v, w: {
            "res": (v["lhs"] if v["cond"] & 1 else v["rhs"]) % (1 << w["res"])
        },
        input_names=["cond", "lhs", "rhs"],
        output_names=["res"],
        width_combos=combos,
        control_case=((1, 3, 5, 4), *_controls({"c": 1})),
    )


@pytest.mark.parametrize("kind", ["And", "Or", "Xor"])
def test_bitwise_uint_uint_width_matrix(kind):
    op = getattr(ps, f"{kind}_UInt_UInt")
    width_matrix_case(
        label=f"{kind}_UInt_UInt",
        specs_factory=lambda c: [_uint("lhs", c[0]), _uint("rhs", c[1]), _uint("res", c[2])],
        make_op=lambda w: op("lhs", "rhs", "res"),
        model=lambda v, w: {
            "res": {
                "And": v["lhs"] & v["rhs"],
                "Or": v["lhs"] | v["rhs"],
                "Xor": v["lhs"] ^ v["rhs"],
            }[kind]
            % (1 << w["res"])
        },
        input_names=["lhs", "rhs"],
        output_names=["res"],
        width_combos=COMBOS_2IN,
        control_case=((3, 5, 4), *_controls({"c": 1})),
    )


# ---------------------------------------------------------------------------
# 比较与 flag 算子
# ---------------------------------------------------------------------------


def test_less_sint_sint_width_matrix():
    width_matrix_case(
        label="Less_SInt_SInt",
        specs_factory=lambda c: [_sint("lhs", c[0]), _sint("rhs", c[1]), _bool("flag")],
        make_op=lambda w: ps.Less_SInt_SInt("lhs", "rhs", "flag"),
        model=lambda v, w: {
            "flag": int(
                sign_extend(v["lhs"], w["lhs"]) < sign_extend(v["rhs"], w["rhs"])
            )
        },
        input_names=["lhs", "rhs"],
        output_names=["flag"],
        width_combos=[(1, 1), (2, 3), (3, 5), (4, 4), (5, 2), (8, 8), (63, 2), (64, 64)],
        control_case=((3, 5), *_controls({"c": 1})),
    )


def test_carry_uint_uint_width_matrix():
    width_matrix_case(
        label="Carry_UInt_UInt",
        specs_factory=lambda c: [
            _uint("lhs", c[0]), _uint("rhs", c[1]), _uint("res", c[2]), _bool("flag"),
        ],
        make_op=lambda w: ps.Carry_UInt_UInt("lhs", "rhs", "res", "flag"),
        model=lambda v, w: {
            "res": 0,  # 宽度提供者:不读不写
            "flag": int(v["lhs"] + v["rhs"] >= (1 << w["res"])),
        },
        input_names=["lhs", "rhs"],
        output_names=["res", "flag"],
        width_combos=COMBOS_2IN,
        control_case=((3, 5, 4), *_controls({"c": 1})),
    )


def test_overflow_sint_sint_width_matrix():
    def model(v, w):
        ow = w["res"]
        mask = (1 << ow) - 1
        a = sign_extend(v["lhs"], w["lhs"]) & mask
        b = sign_extend(v["rhs"], w["rhs"]) & mask
        s = (a + b) & mask
        sa, sb, ss = (a >> (ow - 1)) & 1, (b >> (ow - 1)) & 1, (s >> (ow - 1)) & 1
        return {"res": 0, "flag": int(sa == sb and ss != sa)}

    width_matrix_case(
        label="Overflow_SInt_SInt",
        specs_factory=lambda c: [
            _sint("lhs", c[0]), _sint("rhs", c[1]), _uint("res", c[2]), _bool("flag"),
        ],
        make_op=lambda w: ps.Overflow_SInt_SInt("lhs", "rhs", "res", "flag"),
        model=model,
        input_names=["lhs", "rhs"],
        output_names=["res", "flag"],
        width_combos=COMBOS_2IN,
        control_case=((3, 5, 4), *_controls({"c": 1})),
    )


def test_mul_overflow_uint_uint_width_matrix():
    width_matrix_case(
        label="MulOverflow_UInt_UInt",
        specs_factory=lambda c: [
            _uint("lhs", c[0]), _uint("rhs", c[1]), _uint("res", c[2]), _bool("flag"),
        ],
        make_op=lambda w: ps.MulOverflow_UInt_UInt("lhs", "rhs", "res", "flag"),
        model=lambda v, w: {
            "res": 0,  # 宽度提供者:不读不写
            "flag": int(v["lhs"] * v["rhs"] >= (1 << w["res"])),
        },
        input_names=["lhs", "rhs"],
        output_names=["res", "flag"],
        width_combos=COMBOS_2IN,
        control_case=((3, 5, 4), *_controls({"c": 1})),
    )


def test_is_zero_uint_width_matrix():
    width_matrix_case(
        label="IsZero_UInt",
        specs_factory=lambda c: [_uint("reg", c[0]), _bool("flag")],
        make_op=lambda w: ps.IsZero_UInt("reg", "flag"),
        model=lambda v, w: {"flag": int(v["reg"] == 0)},
        input_names=["reg"],
        output_names=["flag"],
        width_combos=[(1,), (3,), (5,), (8,), (63,), (64,)],
        control_case=((5,), *_controls({"c": 1})),
    )


def test_negative_sint_width_matrix():
    width_matrix_case(
        label="Negative_SInt",
        specs_factory=lambda c: [_sint("reg", c[0]), _bool("flag")],
        make_op=lambda w: ps.Negative_SInt("reg", "flag"),
        model=lambda v, w: {"flag": int(sign_extend(v["reg"], w["reg"]) < 0)},
        input_names=["reg"],
        output_names=["flag"],
        width_combos=[(1,), (3,), (5,), (8,), (63,), (64,)],
        control_case=((5,), *_controls({"c": 1})),
    )


# ---------------------------------------------------------------------------
# 存量算子的约定重验
# ---------------------------------------------------------------------------


def test_add_uint_uint_revalidated():
    width_matrix_case(
        label="Add_UInt_UInt",
        specs_factory=lambda c: [_uint("lhs", c[0]), _uint("rhs", c[1]), _uint("res", c[2])],
        make_op=lambda w: ps.Add_UInt_UInt("lhs", "rhs", "res"),
        model=lambda v, w: {"res": (v["lhs"] + v["rhs"]) % (1 << w["res"])},
        input_names=["lhs", "rhs"],
        output_names=["res"],
        width_combos=COMBOS_2IN,
        control_case=((3, 5, 4), *_controls({"c": 1})),
    )


def test_compare_uint_uint_revalidated():
    width_matrix_case(
        label="Compare_UInt_UInt",
        specs_factory=lambda c: [
            _uint("lhs", c[0]), _uint("rhs", c[1]), _bool("less"), _bool("equal"),
        ],
        make_op=lambda w: ps.Compare_UInt_UInt("lhs", "rhs", "less", "equal"),
        model=lambda v, w: {
            "less": int(v["lhs"] < v["rhs"]),
            "equal": int(v["lhs"] == v["rhs"]),
        },
        input_names=["lhs", "rhs"],
        output_names=["less", "equal"],
        width_combos=COMBOS_2IN,
        control_case=((3, 5), *_controls({"c": 1})),
    )


def test_assign_revalidated():
    width_matrix_case(
        label="Assign",
        specs_factory=lambda c: [_uint("src", c[0]), _uint("dst", c[1])],
        make_op=lambda w: ps.Assign("src", "dst"),
        model=lambda v, w: {"dst": v["src"] % (1 << w["dst"])},
        input_names=["src"],
        output_names=["dst"],
        width_combos=COMBOS_1IN,
        control_case=((3, 5), *_controls({"c": 1})),
    )


# ---------------------------------------------------------------------------
# Add_AnyInt_AnyInt_InPlace:AnyInt 槽新语义(原地,不适用 xor-out runner)
# ---------------------------------------------------------------------------


class TestAddAnyIntSlotSemantics:
    @pytest.mark.parametrize(
        "lhs_w,rhs_w,rhs_type",
        [(4, 4, UINT), (6, 3, SINT), (5, 8, SINT), (8, 8, UINT), (6, 3, UINT)],
    )
    def test_signed_extension_and_dagger_roundtrip(self, lhs_w, rhs_w, rhs_type):
        setup_registers([
            _uint("lhs", lhs_w), RegisterSpec("rhs", rhs_type, rhs_w),
        ])
        masks = {"lhs": (1 << lhs_w) - 1, "rhs": (1 << rhs_w) - 1}
        for lhs_val in (0, 1, masks["lhs"], masks["lhs"] // 2, 5):
            for rhs_val in (0, 1, masks["rhs"], masks["rhs"] // 3 + 1, masks["rhs"] - 1):
                state = make_basis_state({"lhs": lhs_val, "rhs": rhs_val})
                op = ps.Add_AnyInt_AnyInt_InPlace("lhs", "rhs")
                op(state)
                extended = (
                    sign_extend(rhs_val, rhs_w)
                    if rhs_type is SINT
                    else rhs_val
                )
                got = dict(zip(("lhs", "rhs"), read_registers(state, ("lhs", "rhs"), masks)))
                assert got["lhs"] == (lhs_val + extended) % (1 << lhs_w), (
                    f"{lhs_w}/{rhs_w}/{rhs_type}: {lhs_val} + {rhs_val}(ext {extended}) "
                    f"-> {got['lhs']}"
                )
                assert got["rhs"] == rhs_val
                op.dag(state)
                assert dict(zip(("lhs", "rhs"), read_registers(state, ("lhs", "rhs"), masks))) == {
                    "lhs": lhs_val, "rhs": rhs_val,
                }

    def test_alias_rejected(self):
        setup_registers([_uint("x", 4)])
        with pytest.raises((ValueError, RuntimeError)):
            ps.Add_AnyInt_AnyInt_InPlace("x", "x")


# ---------------------------------------------------------------------------
# two_complement_decode 自检(模型本身的正确性)
# ---------------------------------------------------------------------------


@pytest.mark.parametrize(
    "value,width,expected",
    [(0, 4, 0), (1, 4, 1), (7, 4, 7), (8, 4, -8), (15, 4, -1), (1, 1, -1), (0, 1, 0)],
)
def test_two_complement_decode(value, width, expected):
    assert two_complement_decode(value, width) == expected
