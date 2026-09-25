# qram_simulator 绑定层的功能冒烟测试。
# 运行：pytest bindings/python/test（需先 pip install .，本地或 CI 均可）
#
# 语义备注（与 C++ 实现保持一致）：
# - QRAMFullAmp::apply 内部调用 qutrit QRAMCircuit::run(version)，后者在
#   未设置噪声时直接抛异常，因此 QRAMFullAmp 的用例均注入近零噪声。

import pytest

import qram_simulator as qs

# 近零噪声：满足 run(version) 的"必须非空"约束，物理上近似无噪演化
TINY_NOISE = {qs.OperationType.Depolarizing: 1e-15}


def test_module_surface():
    # 核心导出齐全（绑定层接口回归护栏）
    for name in (
        "QRAMCircuitQubit",
        "QRAMCircuitQutrit",
        "QRAMFullAmp",
        "TimeStep",
        "Operation",
        "OperationPack",
        "TimeSlices",
        "OperationType",
        "set_seed",
        "get_seed",
        "ARCH_QUBIT",
        "ARCH_QUTRIT",
    ):
        assert hasattr(qs, name), f"缺少导出: {name}"
    assert isinstance(qs.__version__, str)


def test_seed_roundtrip_and_reproducibility():
    qs.set_seed(12345)
    assert qs.get_seed() == 12345

    def run_once():
        qs.set_seed(12345)
        qram = qs.QRAMCircuitQutrit(2, 2)
        qram.set_memory_random()
        qram.set_noise_models({qs.OperationType.Depolarizing: 1e-3})
        qram.set_input_random(8)
        qram.run_normal()
        return qram.sample_and_get_fidelity()

    assert run_once() == run_once()


def test_qubit_noise_free_fidelity():
    qs.set_seed(42)
    qram = qs.QRAMCircuitQubit(3, 2)
    qram.set_memory_random()
    qram.set_input_uniform(20)
    qram.run_normal()
    # 无噪声时采样保真度应回到 1
    assert qram.sample_and_get_fidelity() == pytest.approx(1.0, abs=1e-9)


def test_qutrit_noise_free_fidelity():
    # run_normal 直接调用不走 run(version) 的噪声门控，无噪可用
    qs.set_seed(7)
    qram = qs.QRAMCircuitQutrit(3, 2)
    qram.set_memory_random()
    qram.set_input_uniform(20)
    qram.run_normal()
    assert qram.sample_and_get_fidelity() == pytest.approx(1.0, abs=1e-9)


def test_noise_model_validation():
    qram = qs.QRAMCircuitQubit(2, 1)
    with pytest.raises(RuntimeError):
        qram.set_noise_models({qs.OperationType.Depolarizing: 1.5})
    with pytest.raises(RuntimeError):
        qram.set_noise_models({qs.OperationType.Damping: 1.0})
    assert qram.is_noise_free()

    qram.set_noise_models({qs.OperationType.Depolarizing: 1e-3})
    assert not qram.is_noise_free()
    assert not qram.has_damping()

    qram.set_noise_models({qs.OperationType.Depolarizing: 1e-3,
                           qs.OperationType.Damping: 1e-4})
    assert qram.has_damping()


def test_qubit_noisy_fidelity_in_range():
    qs.set_seed(11)
    qram = qs.QRAMCircuitQubit(3, 2)
    qram.set_memory_random()
    qram.set_noise_models({
        qs.OperationType.Depolarizing: 1e-3,
        qs.OperationType.Damping: 1e-4,
    })
    qram.set_input_uniform(20)
    qram.run_normal()
    fidelity = qram.sample_and_get_fidelity()
    assert 0.0 <= fidelity <= 1.0 + 1e-9


def test_qubit_run_versions_consistent():
    # 剪枝运行与不剪枝基准在无噪声下应给出一致结果
    def run(fn):
        qs.set_seed(2024)
        qram = qs.QRAMCircuitQubit(2, 2)
        qram.set_memory_random()
        qram.set_input_uniform(10)
        fn(qram)
        return qram.sample_and_get_fidelity()

    assert run(lambda q: q.run_normal()) == pytest.approx(
        run(lambda q: q.run_full()), abs=1e-12)
    assert run(lambda q: q.run("normal")) == pytest.approx(
        run(lambda q: q.run("full")), abs=1e-12)


def test_time_step_schedule():
    ts = qs.TimeStep(2, 1)
    assert ts.full_step() > 0

    noise_free = ts.generate({}, qs.ARCH_QUBIT)
    assert len(noise_free) > 0
    assert str(noise_free)  # 可打印

    noisy = ts.generate({qs.OperationType.Depolarizing: 1e-3}, qs.ARCH_QUTRIT)
    assert len(noisy) > 0
    assert len(noisy.time_slices) == len(noisy)

    pack = ts.generate_step(1)
    assert isinstance(pack, qs.OperationPack)
    assert ts.is_bad_branch(0) in (True, False)
    lo, hi = ts.get_bad_range_qubit(0)
    assert 0 <= lo <= hi


def test_full_amp_zero_address_identity():
    # 地址 |0…0> 装载 mem[0]=0 → 态不变（与比特序约定无关的精确断言）
    qs.set_seed(5)
    manipulator = qs.QRAMFullAmp(2, 2, [0, 0, 0, 0])
    manipulator.set_noise_models(TINY_NOISE)
    state = [0.0j] * 16
    state[0] = 1.0 + 0.0j
    out = manipulator.apply(state, [0, 1], [2, 3], [], version="normal")
    assert len(out) == 16
    for got, expect in zip(out, state):
        assert got == pytest.approx(expect, abs=1e-6)


def test_full_amp_uniform_address_loading():
    # |a>|00> 均匀叠加（addr 比特 {0,1}，data 比特 {2,3}，小端约定）
    # + memory=[0,1,2,3] → 装载后 4 个基矢各 1/4 概率，data 字互不相同
    qs.set_seed(5)
    manipulator = qs.QRAMFullAmp(2, 2, [0, 1, 2, 3])
    manipulator.set_noise_models(TINY_NOISE)
    state = [0.0j] * 16
    for addr in range(4):
        state[addr] = 0.5 + 0.0j  # data 比特（高位）为 0
    out = manipulator.apply(state, [0, 1], [2, 3], [], version="normal")

    probs = [abs(c) ** 2 for c in out]
    assert sum(probs) == pytest.approx(1.0, abs=1e-9)
    nonzero = sorted(p for p in probs if p > 1e-6)
    assert nonzero == pytest.approx([0.25, 0.25, 0.25, 0.25], abs=1e-6)
    nonzero_idx = [i for i, p in enumerate(probs) if p > 1e-6]
    data_words = {i >> 2 for i in nonzero_idx}
    assert len(data_words) == 4  # 四个地址装载出四个互不相同的 data 字


def test_full_amp_deterministic_given_seed():
    # 同种子 + 新实例 → apply 结果逐位一致（内部采样走全局随机引擎）
    state = [0.0j] * 16
    for addr in range(4):
        state[addr] = 0.5 + 0.0j

    outs = []
    for _ in range(2):
        qs.set_seed(99)
        manipulator = qs.QRAMFullAmp(2, 2, [1, 0, 3, 2])
        manipulator.set_noise_models({qs.OperationType.Depolarizing: 1e-3})
        outs.append(manipulator.apply(state, [0, 1], [2, 3], [], version="normal"))

    assert outs[0] == outs[1]
