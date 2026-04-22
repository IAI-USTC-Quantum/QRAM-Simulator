"""
PySparQ 测试共享配置和 Fixture。

提供：
- fresh_system: 自动清理 System 状态的 fixture
- pytest marker 配置

Helper 函数：
- get_amplitude: 获取特定基态的振幅
- get_reg_value: 获取寄存器值
- verify_unitarity_self_adjoint: 验证自伴算子幺正性
- verify_unitarity_explicit_dag: 验证显式 dag 算子幺正性
"""

import pytest
import numpy as np


def pytest_configure(config):
    """注册自定义 pytest markers。"""
    config.addinivalue_line(
        "markers", "slow: marks tests as slow (deselect with '-m \"not slow\"')"
    )


@pytest.fixture(autouse=True)
def fresh_system():
    """每个测试前后自动清理 System 状态。

    这确保测试之间相互隔离，避免状态污染。
    """
    import pysparq as ps

    ps.System.clear()
    yield
    ps.System.clear()


@pytest.fixture
def small_qram():
    """创建小型 QRAM 用于快速测试。

    Returns:
        Callable: 接受 (memory, n_bits, data_size) 参数，返回 QRAMCircuit_qutrit
    """

    def _create(memory: list, n_bits: int = None, data_size: int = 8):
        import pysparq as ps
        import math

        if n_bits is None:
            n_bits = int(math.log2(len(memory))) + 1 if len(memory) > 0 else 1
        return ps.QRAMCircuit_qutrit(n_bits, data_size, memory)

    return _create


# ---------------------------------------------------------------------------
# Helper Functions
# ---------------------------------------------------------------------------


def get_amplitude(state, reg_values: dict) -> complex:
    """获取特定基态的振幅。

    Args:
        state: SparseState 实例
        reg_values: 字典，映射寄存器名称到期望值

    Returns:
        匹配基态的复数振幅，如果未找到返回 0

    Example:
        >>> amp = get_amplitude(state, {"addr": 3, "data": 8})
    """
    import pysparq as ps

    for basis in state.basis_states:
        match = True
        for reg_name, expected_val in reg_values.items():
            reg_id = ps.System.get_id(reg_name)
            if basis.get(reg_id).value != expected_val:
                match = False
                break
        if match:
            return basis.amplitude
    return complex(0, 0)


def get_reg_value(basis, reg: str | int) -> int:
    """获取基态中寄存器的值。

    Args:
        basis: System 基态（来自 state.basis_states）
        reg: 寄存器名称或 ID

    Returns:
        寄存器中存储的整数值
    """
    import pysparq as ps

    if isinstance(reg, str):
        reg_id = ps.System.get_id(reg)
    else:
        reg_id = reg
    return basis.get(reg_id).value


def verify_unitarity_self_adjoint(
    state,
    operator,
    rtol: float = 1e-6,
    atol: float = 1e-10,
) -> bool:
    """验证自伴算子的幺正性：U * U = I。

    自伴算子满足 U^dagger = U，因此应用两次应返回初始状态。

    Args:
        state: SparseState 实例
        operator: 具有操作符接口的对象
        rtol: 相对容差（未使用，保留兼容）
        atol: 绝对容差

    Returns:
        True 如果验证通过

    Example:
        >>> state = ps.SparseState()
        >>> # ... 初始化状态 ...
        >>> assert verify_unitarity_self_adjoint(state, operator)
    """
    import pysparq as ps

    # 记录初始振幅
    initial_amplitudes = {}
    for basis in state.basis_states:
        # 使用寄存器值作为键
        key = tuple(
            (ps.System.name_of(i), basis.get(i).value)
            for i in range(len(basis.registers))
        )
        initial_amplitudes[key] = basis.amplitude

    # 应用操作符两次
    operator(state)
    operator(state)

    # 验证返回初始状态
    for basis in state.basis_states:
        key = tuple(
            (ps.System.name_of(i), basis.get(i).value)
            for i in range(len(basis.registers))
        )
        expected = initial_amplitudes.get(key, complex(0, 0))
        if abs(basis.amplitude - expected) > atol:
            return False
    return True


def verify_unitarity_explicit_dag(
    state,
    operator,
    rtol: float = 1e-6,
    atol: float = 1e-10,
) -> bool:
    """验证非自伴算子的幺正性：U^dagger * U = I。

    使用显式的 dag() 方法验证。

    Args:
        state: SparseState 实例
        operator: 具有 dag() 方法的操作符对象
        rtol: 相对容差（未使用，保留兼容）
        atol: 绝对容差

    Returns:
        True 如果验证通过

    Example:
        >>> state = ps.SparseState()
        >>> # ... 初始化状态 ...
        >>> assert verify_unitarity_explicit_dag(state, operator)
    """
    import pysparq as ps

    # 记录初始振幅
    initial_amplitudes = {}
    for basis in state.basis_states:
        key = tuple(
            (ps.System.name_of(i), basis.get(i).value)
            for i in range(len(basis.registers))
        )
        initial_amplitudes[key] = basis.amplitude

    # 应用前向操作后应用 dag
    operator(state)
    operator.dag(state)

    # 验证返回初始状态
    for basis in state.basis_states:
        key = tuple(
            (ps.System.name_of(i), basis.get(i).value)
            for i in range(len(basis.registers))
        )
        expected = initial_amplitudes.get(key, complex(0, 0))
        if abs(basis.amplitude - expected) > atol:
            return False
    return True


def state_to_amplitude_dict(state, main_reg: str) -> dict:
    """将稀疏状态转换为振幅字典。

    Args:
        state: SparseState 实例
        main_reg: 主寄存器名称

    Returns:
        字典映射寄存器值到振幅
    """
    import pysparq as ps

    reg_id = ps.System.get_id(main_reg)
    return {basis.get(reg_id).value: basis.amplitude for basis in state.basis_states}


def assert_amplitude_close(actual: complex, expected: complex, tol: float = 1e-10):
    """验证振幅接近预期值。

    Args:
        actual: 实际振幅
        expected: 预期振幅
        tol: 容差

    Raises:
        AssertionError: 如果振幅不匹配
    """
    assert abs(actual - expected) < tol, f"振幅不匹配: {actual} != {expected}"


def assert_probability_distribution(
    state,
    expected_probs: dict,
    main_reg: str,
    rtol: float = 0.1,
    atol: float = 0.05,
):
    """验证测量概率分布接近预期。

    Args:
        state: SparseState 实例
        expected_probs: 字典映射寄存器值到预期概率
        main_reg: 主寄存器名称
        rtol: 相对容差
        atol: 绝对容差

    Raises:
        AssertionError: 如果概率不匹配
    """
    import pysparq as ps

    reg_id = ps.System.get_id(main_reg)
    measured = {}
    for basis in state.basis_states:
        val = basis.get(reg_id).value
        measured[val] = measured.get(val, 0) + abs(basis.amplitude) ** 2

    for val, expected_prob in expected_probs.items():
        actual_prob = measured.get(val, 0)
        if not np.isclose(actual_prob, expected_prob, rtol=rtol, atol=atol):
            raise AssertionError(
                f"概率不匹配: 值 {val} 预期 {expected_prob:.4f}, 实际 {actual_prob:.4f}"
            )
