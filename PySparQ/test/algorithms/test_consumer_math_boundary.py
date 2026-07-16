"""Independent analytic checks for the consumed QDA/CKS helper boundary."""

from __future__ import annotations

import math

import numpy as np

from pysparq.algorithms.cks_solver import (
    ChebyshevPolynomialCoefficient,
    get_coef_common,
    get_coef_positive_only,
)
from pysparq.algorithms.qda_solver import compute_fs, compute_rotation_matrix
from pysparq.algorithms.qram_utils import make_vector_tree, scale_and_convert_vector


def test_qda_schedule_matches_closed_form_independently():
    for kappa in (2.0, 7.0, 31.0):
        for p in (0.25, 0.5, 0.75):
            for s in (0.0, 0.125, 0.5, 0.875, 1.0):
                expected = kappa / (kappa - 1) * (
                    1 - (1 + s * (kappa ** (p - 1) - 1)) ** (1 / (1 - p))
                )
                assert math.isclose(compute_fs(s, kappa, p), expected, abs_tol=1e-13)


def test_qda_rotation_is_the_normalized_interpolation_reflection():
    for fs in np.linspace(0.0, 1.0, 17):
        theta = math.atan2(fs, 1 - fs)
        expected = np.array(
            [[math.cos(theta), math.sin(theta)], [math.sin(theta), -math.cos(theta)]]
        )
        actual = np.asarray(compute_rotation_matrix(float(fs))).reshape(2, 2).real
        assert np.allclose(actual, expected, atol=1e-13)
        assert np.allclose(actual @ actual, np.eye(2), atol=1e-13)


def test_qram_preparation_helpers_match_direct_integer_model():
    matrix = np.array([[0.25, -0.5], [0.75, -1.0]])
    converted = scale_and_convert_vector(matrix.ravel(), 3, 8, from_matrix=True)
    expected_converted = [2, 6, 252, 248]
    assert converted == expected_converted

    signed = [value if value < 128 else value - 256 for value in converted]
    expected_tree = [
        sum(value * value for value in signed),
        signed[0] ** 2 + signed[1] ** 2,
        signed[2] ** 2 + signed[3] ** 2,
        *converted,
        0,
    ]
    assert make_vector_tree(converted, 8) == expected_tree


def test_cks_coefficients_match_binomial_tail_definition():
    for b in range(1, 9):
        coeffs = ChebyshevPolynomialCoefficient(b)
        for j in range(b):
            expected = 4 * sum(
                math.comb(2 * b, b + i) / (4**b) for i in range(j + 1, b + 1)
            )
            assert math.isclose(coeffs.coef(j), expected, rel_tol=0, abs_tol=1e-15)
            assert coeffs.step(j) == 2 * j + 1
            assert coeffs.sign(j) is bool(j & 1)


def test_cks_rotation_helpers_match_direct_signed_models():
    width = 5
    positive_max = (1 << width) - 1
    signed_max = (1 << (width - 1)) - 1

    for value in range(1 << width):
        x = math.sqrt(value / positive_max)
        y = math.sqrt(1 - value / positive_max)
        assert np.allclose(
            np.asarray(get_coef_positive_only(width, value, 0, 0)).reshape(2, 2),
            [[x, -y], [y, x]],
        )

        signed = value if value < (1 << (width - 1)) else value - (1 << width)
        if signed == -(1 << (width - 1)):
            # SparseMatrix.from_dense clips to the symmetric encodable range
            # [-signed_max, signed_max], so this raw two's-complement code is
            # outside the accepted CKS boundary.
            continue
        magnitude = math.sqrt(abs(signed) / signed_max)
        complement = math.sqrt(1 - abs(signed) / signed_max)
        if signed >= 0:
            expected = [[magnitude, -complement], [complement, magnitude]]
        else:
            expected = [[-1j * magnitude, complement], [complement, -1j * magnitude]]
        assert np.allclose(
            np.asarray(get_coef_common(width, value, 0, 1)).reshape(2, 2), expected
        )
