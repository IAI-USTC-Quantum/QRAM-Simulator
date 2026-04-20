"""Classical utility functions for QRAM circuit construction.

Ported from the C++ SparQ codebase:
  - ``Common/include/basic.h`` (pow2, make_complement, get_complement)
  - ``SparQ_Algorithm/include/BlockEncoding/make_qram.h`` (column_flatten,
    scaleAndConvertVector, make_vector_tree)
  - ``SparQ/include/condrot.h`` (make_func, make_func_inv)

These are pure-classical helpers -- no quantum state or pysparq imports needed.
"""

from __future__ import annotations

import math
from typing import Union

import numpy as np

# ---------------------------------------------------------------------------
# Constants
# ---------------------------------------------------------------------------

PI: float = math.pi
"""Pi constant matching the C++ ``constexpr double pi``."""

# ---------------------------------------------------------------------------
# Bit helpers
# ---------------------------------------------------------------------------


def pow2(n: int) -> int:
    """Return ``2**n`` using a left shift, matching ``basic.h`` semantics.

    Args:
        n: The exponent.

    Returns:
        2 raised to the power *n*.

    Examples:
        >>> pow2(0)
        1
        >>> pow2(10)
        1024
    """
    return 1 << n


# ---------------------------------------------------------------------------
# Two's-complement helpers
# ---------------------------------------------------------------------------


def make_complement(data: int, data_sz: int) -> int:
    """Convert a signed integer to its two's-complement representation.

    For negative *data* with *data_sz* < 64 bits, the result is
    ``2**data_sz + data`` (i.e. the unsigned encoding of the negative value
    in *data_sz* bits).  For non-negative data, or when ``data_sz == 64``,
    *data* is returned unchanged.

    Ported from ``Common/include/basic.h:100-106``.

    Args:
        data: Signed integer value.
        data_sz: Bit-width of the target representation.

    Returns:
        Unsigned integer in two's-complement form.

    Examples:
        >>> make_complement(3, 8)
        3
        >>> make_complement(-3, 8)
        253
        >>> make_complement(-1, 4)
        15
    """
    if data_sz == 64 or data >= 0:
        return data
    return pow2(data_sz) + data


def get_complement(data: int, data_sz: int) -> int:
    """Reverse two's-complement: sign-extend an unsigned value to a signed int.

    If *data_sz* is 0 the result is 0.  Otherwise the value is shifted left
    so that its sign bit lands in the MSB of a Python int (arbitrary width),
    then arithmetic-right-shifted back, achieving sign extension.

    Ported from ``Common/include/basic.h:108-111``:

    .. code-block:: c++

        return data_sz ? (int64_t)(data << (64 - data_sz)) >> (64 - data_sz) : 0;

    In Python we emulate the 64-bit arithmetic shift by clamping the working
    width to 64 bits and using bit-level sign extension.

    Args:
        data: Unsigned integer in two's-complement encoding.
        data_sz: Bit-width that was used for the encoding.

    Returns:
        The corresponding signed integer.

    Examples:
        >>> get_complement(253, 8)
        -3
        >>> get_complement(15, 4)
        -1
        >>> get_complement(0, 4)
        0
    """
    if data_sz == 0:
        return 0
    if data_sz == 64:
        # For 64-bit, interpret data as a signed 64-bit integer directly.
        if data >= (1 << 63):
            return data - (1 << 64)
        return data
    # Sign bit is bit (data_sz - 1).
    sign_bit = 1 << (data_sz - 1)
    if data & sign_bit:
        # Negative: sign-extend by subtracting 2**data_sz.
        return data - pow2(data_sz)
    return data


# ---------------------------------------------------------------------------
# Matrix / vector helpers for QRAM data preparation
# ---------------------------------------------------------------------------


def column_flatten(row_vec: list[float]) -> list[float]:
    """Transpose a flat row-major square-matrix representation to column-major.

    The input *row_vec* must have length ``n*n`` for some integer *n*.  The
    output rearranges ``row_vec[i*n + j]`` to ``col_vec[j*n + i]``, which is
    equivalent to a standard matrix transpose.

    Ported from ``SparQ_Algorithm/include/BlockEncoding/make_qram.h``,
    ``get_column_flatten``.

    Args:
        row_vec: Flat row-major representation of a square matrix.

    Returns:
        Flat column-major representation of the same matrix.

    Raises:
        ValueError: If the length of *row_vec* is not a perfect square.

    Examples:
        >>> column_flatten([1, 2, 3, 4])
        [1, 3, 2, 4]
    """
    size = len(row_vec)
    n = int(math.isqrt(size))
    if n * n != size:
        raise ValueError(
            f"Input length {size} is not a perfect square; "
            "expected a flat square-matrix representation."
        )
    col_vec = [0.0] * size
    for i in range(n):
        for j in range(n):
            col_vec[j * n + i] = row_vec[i * n + j]
    return col_vec


def scale_and_convert_vector(
    input_vec: Union[list[float], np.ndarray],
    exponent: int,
    data_size: int,
    from_matrix: bool = True,
) -> list[int]:
    """Scale floating-point values and convert to two's-complement integers.

    When *from_matrix* is ``True`` (the default), the input is treated as a
    flat row-major square matrix and is first transposed to column-major via
    :func:`column_flatten`.  Each element is then multiplied by
    ``2**exponent``, rounded to the nearest integer, and encoded with
    :func:`make_complement` at *data_size* bits.

    Ported from ``SparQ_Algorithm/include/BlockEncoding/make_qram.h``,
    ``scaleAndConvertVector``.

    Args:
        input_vec: Flattened matrix (or plain vector) of floating-point values.
        exponent: Power-of-two scaling factor (``scale = 2**exponent``).
        data_size: Bit-width for two's-complement encoding.
        from_matrix: If ``True``, transpose input before scaling.

    Returns:
        List of unsigned integers in two's-complement encoding.

    Examples:
        >>> scale_and_convert_vector([0.25, -0.25, 0.25, -0.25],
        ...                          exponent=4, data_size=8,
        ...                          from_matrix=False)
        [4, 252, 4, 252]
    """
    if isinstance(input_vec, np.ndarray):
        input_vec = input_vec.tolist()
    if from_matrix:
        col_vec = column_flatten(input_vec)
    else:
        col_vec = list(input_vec)
    scale = 2.0 ** exponent
    output: list[int] = []
    for value in col_vec:
        scaled = int(round(value * scale))
        output.append(make_complement(scaled, data_size))
    return output


def make_vector_tree(dist: list[int], data_size: int) -> list[int]:
    """Build a binary tree from leaf distribution data for QRAM circuits.

    The algorithm iteratively pairs adjacent entries in *dist*:

    - **Leaf level** (first iteration): each pair is squared after
      sign-extension via :func:`get_complement`:

      ``get_complement(a)**2 + get_complement(b)**2``

    - **Inner levels** (subsequent iterations): pairs are summed directly.

    After pairing, the current layer is appended, producing a breadth-first
    tree ordering.  A trailing ``0`` is appended to match the C++ behaviour.

    Ported from ``SparQ_Algorithm/include/BlockEncoding/make_qram.h``,
    ``make_vector_tree``.

    Args:
        dist: Leaf-level distribution values (unsigned two's-complement).
        data_size: Bit-width used for sign-extension at the leaf level.

    Returns:
        Binary tree stored as a flat list (breadth-first), with a trailing 0.

    Examples:
        >>> make_vector_tree([0, 0, 0, 0], data_size=4)
        [0, 0, 0, 0, 0]
    """
    dist_sz = len(dist)
    temp_tree = list(dist)

    current_sz = dist_sz
    is_first = True

    while True:
        temp: list[int] = []
        i = 0
        while i < current_sz:
            if i + 1 < current_sz:
                if is_first:
                    # Leaf nodes: squared values with sign extension.
                    a = get_complement(temp_tree[i], data_size)
                    b = get_complement(temp_tree[i + 1], data_size)
                    temp.append(a * a + b * b)
                else:
                    # Inner nodes: sum directly.
                    temp.append(temp_tree[i] + temp_tree[i + 1])
            i += 2

        # Combine: new layer prepended, then the existing tree.
        temp.extend(temp_tree)
        temp_tree = temp

        # Update for next layer (ceiling division by 2).
        current_sz = (current_sz + 1) // 2
        is_first = False

        if current_sz <= 1:
            break

    temp_tree.append(0)
    return temp_tree


# ---------------------------------------------------------------------------
# Rotation-matrix helpers (conditional rotations)
# ---------------------------------------------------------------------------


def make_func(value: int, n_digit: int) -> list[complex]:
    """Compute a 2x2 rotation matrix from a rational register value.

    The rotation angle is:

    ``theta = value / 2**n_digit * 2 * pi``

    (with a special case for ``n_digit == 64`` where the denominator becomes
    ``2 * 2**63`` to avoid overflow, matching the C++ implementation).

    The returned matrix is row-major: ``[cos(theta), -sin(theta),
    sin(theta), cos(theta)]``.

    Ported from ``SparQ/include/condrot.h:18-34``.

    Args:
        value: The integer register value (unsigned).
        n_digit: The bit-width of the register.

    Returns:
        A list of four complex numbers representing the 2x2 rotation matrix.

    Examples:
        >>> import math
        >>> mat = make_func(0, 4)
        >>> [round(c.real, 6) for c in mat]
        [1.0, -0.0, 0.0, 1.0]
    """
    if n_digit == 64:
        theta = value * 1.0 / 2 / pow2(63)
    else:
        theta = value * 1.0 / pow2(n_digit)

    theta *= 2 * PI

    u00 = complex(math.cos(theta), 0.0)
    u01 = complex(-math.sin(theta), 0.0)
    u10 = complex(math.sin(theta), 0.0)
    u11 = complex(math.cos(theta), 0.0)

    return [u00, u01, u10, u11]


def make_func_inv(value: int, n_digit: int) -> list[complex]:
    """Compute the inverse 2x2 rotation matrix from a rational register value.

    Same as :func:`make_func` but the off-diagonal signs are flipped:

    ``[cos(theta), sin(theta), -sin(theta), cos(theta)]``.

    Ported from ``SparQ/include/condrot.h:42-58``.

    Args:
        value: The integer register value (unsigned).
        n_digit: The bit-width of the register.

    Returns:
        A list of four complex numbers representing the inverse rotation matrix.

    Examples:
        >>> import math
        >>> mat = make_func_inv(0, 4)
        >>> [round(c.real, 6) for c in mat]
        [1.0, 0.0, -0.0, 1.0]
    """
    if n_digit == 64:
        theta = value * 1.0 / 2 / pow2(63)
    else:
        theta = value * 1.0 / pow2(n_digit)

    theta *= 2 * PI

    u00 = complex(math.cos(theta), 0.0)
    u01 = complex(math.sin(theta), 0.0)
    u10 = complex(-math.sin(theta), 0.0)
    u11 = complex(math.cos(theta), 0.0)

    return [u00, u01, u10, u11]


# ---------------------------------------------------------------------------
# Demo
# ---------------------------------------------------------------------------


def create_qram_utils_demo() -> str:
    """Return a demo script string showing typical usage of this module.

    Returns:
        A multi-line Python source string that exercises all public helpers.

    Examples:
        >>> script = create_qram_utils_demo()
        >>> isinstance(script, str)
        True
    """
    return '''\
"""Demo: classical QRAM utility functions."""

from pysparq.algorithms.qram_utils import (
    pow2,
    make_complement,
    get_complement,
    column_flatten,
    scale_and_convert_vector,
    make_vector_tree,
    make_func,
    make_func_inv,
)

# --- Bit helpers ---
print("pow2(10) =", pow2(10))           # 1024

# --- Two's complement ---
enc = make_complement(-3, 8)
print(f"make_complement(-3, 8) = {enc}")  # 253
dec = get_complement(enc, 8)
print(f"get_complement({enc}, 8) = {dec}")  # -3

# --- Matrix transpose ---
row_major = [1.0, 2.0, 3.0, 4.0]  # [[1,2],[3,4]]
col_major = column_flatten(row_major)
print(f"column_flatten({row_major}) = {col_major}")  # [1,3,2,4]

# --- Scale and convert ---
vec = [0.25, -0.25, 0.25, -0.25]
result = scale_and_convert_vector(vec, exponent=4, data_size=8, from_matrix=False)
print(f"scale_and_convert_vector: {result}")

# --- Vector tree ---
tree = make_vector_tree([0, 0, 0, 0], data_size=4)
print(f"make_vector_tree: {tree}")

# --- Rotation matrices ---
mat = make_func(1, 4)  # theta = 1/16 * 2*pi
print(f"make_func(1, 4): {[round(c.real, 4) for c in mat]}")

mat_inv = make_func_inv(1, 4)
print(f"make_func_inv(1, 4): {[round(c.real, 4) for c in mat_inv]}")
'''
