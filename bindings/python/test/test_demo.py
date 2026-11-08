"""Exercise the installed standalone product through native evolution and HTTP."""
import hashlib
import json
import random
import re
import subprocess
import sys
import threading
import urllib.error
import urllib.request

import pytest

from qram_simulator.demo import Experiment, run
from qram_simulator.demo_server import make_server


@pytest.mark.parametrize("channel", ["none", "BitFlip", "PhaseFlip", "Depolarizing", "Damping"])
def test_pruned_observables_match_full_without_consuming_rng(channel):
    import qram_simulator as qs
    memory = [1, 0, 1, 0, 0, 1, 0, 1]
    noise = {} if channel == "none" else {getattr(qs.OperationType, channel): 0.04}
    for seed in range(8):
        states = []
        for pruned in (False, True):
            qram = qs.QRAMCircuitQubit(3, 1, memory)
            qram.set_input_addresses([0, 2, 5, 7])
            qram.set_noise_models(noise)
            qs.set_seed(20261109+seed)
            (qram.run_normal if pruned else qram.run_full)()
            qram.prepare_observables()
            first = qram.get_output_distribution()
            conv = qram.get_fidelity_conventions()
            qram.prepare_observables()
            assert first == qram.get_output_distribution()
            post = qram.sample_and_get_fidelity()
            states.append((first, conv, post))
        assert states[0][0] == pytest.approx(states[1][0], abs=1e-12)
        assert states[0][1] == pytest.approx(states[1][1], abs=1e-12)
        assert states[0][2] == pytest.approx(states[1][2], abs=1e-12)


@pytest.mark.parametrize("addresses", [[], [0, 0], [8]])
def test_address_input_rejects_invalid_columns(addresses):
    import qram_simulator as qs
    qram = qs.QRAMCircuitQubit(3, 1)
    with pytest.raises(ValueError):
        qram.set_input_addresses(addresses)


def test_noiseless_native_ensemble_and_exact_memory_digest():
    config = Experiment(depth=2, inputs=4, trajectories=3, channel="none", probability=0)
    result = run(config)
    rng = random.Random(config.seed)
    expected = bytes(rng.randrange(2) for _ in range(4))
    assert result["memory"]["sha256"] == hashlib.sha256(expected).hexdigest()
    assert result["resources"]["routing_qubits"] == 6
    assert result["fidelity"]["samples"] == pytest.approx([1.0]*3, abs=1e-12)
    assert result["scope"]["input_coverage"] == "all addresses"
    assert result["scope"]["acceptance_certificate"] is False


@pytest.mark.parametrize("channel", ["BitFlip", "PhaseFlip", "Depolarizing", "Damping"])
def test_actual_noise_channels_trace_and_reproducibility(channel):
    config = Experiment(depth=2, inputs=4, trajectories=16,
                        channel=channel, probability=0.1)
    first, second = run(config), run(config)
    assert first["fidelity"]["samples"] == second["fidelity"]["samples"]
    assert first["normalization"]["max_trace_error"] < 1e-12
    assert 0 <= first["fidelity"]["mean"] < 1
    lo, hi = first["fidelity"]["confidence"]["interval"]
    assert 0 <= lo <= first["fidelity"]["mean"] <= hi <= 1


@pytest.mark.parametrize("kwargs", [dict(depth=0), dict(depth=21), dict(inputs=0),
    dict(depth=1, inputs=16), dict(trajectories=True), dict(trajectories=0),
    dict(probability=float("nan")), dict(probability=1), dict(seed=-1),
    dict(channel="none", probability=0.1), dict(channel="wrong")])
def test_invalid_experiments_fail_before_native_execution(kwargs):
    with pytest.raises(ValueError):
        run(Experiment(**kwargs))


def test_real_cli_writes_artifact(tmp_path):
    target = tmp_path/"experiment.json"
    child = subprocess.run([sys.executable, "-m", "qram_simulator.demo", "run",
        "--depth", "2", "--inputs", "4", "--trajectories", "2",
        "--channel", "none", "--probability", "0", "--out", str(target)],
        capture_output=True, text=True, check=True)
    assert not child.stdout
    result = json.loads(target.read_text())
    assert result["schema"] == "qramsim.demo.v1"
    assert result["fidelity"]["mean"] == pytest.approx(1, abs=1e-12)


def test_gui_session_and_real_isolated_native_worker():
    with make_server() as server:
        thread = threading.Thread(target=server.serve_forever, daemon=True)
        thread.start()
        base = f"http://127.0.0.1:{server.server_port}"
        try:
            with urllib.request.urlopen(base) as response:
                html = response.read().decode()
                assert "frame-ancestors 'none'" in response.headers["Content-Security-Policy"]
            token = re.search(r"const token='([^']+)'", html).group(1)
            payload = json.dumps(dict(depth=2, inputs=4, trajectories=2,
                                      channel="none", probability=0)).encode()
            def post(headers):
                req = urllib.request.Request(base+"/api/run", data=payload,
                    headers={"Content-Type": "application/json", **headers})
                return urllib.request.urlopen(req)
            with pytest.raises(urllib.error.HTTPError) as missing:
                post({})
            assert missing.value.code == 403
            with pytest.raises(urllib.error.HTTPError) as foreign:
                post({"X-Session": token, "Origin": "https://example.invalid"})
            assert foreign.value.code == 403
            with post({"X-Session": token}) as response:
                result = json.load(response)
            assert result["fidelity"]["mean"] == pytest.approx(1, abs=1e-12)
            assert result["software"]["gpu_execution"] is False
            with urllib.request.urlopen(base+"/api/info") as response:
                assert json.load(response)["software"] == "QRAM-Simulator"
        finally:
            server.shutdown()
            thread.join(timeout=3)
