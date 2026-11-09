"""Audit the installed native wheel, then exercise its CLI and local GUI.

cibuildwheel runs this after repair/install in a clean test environment.
Use --notices-only when a separate step already runs the complete test suite.
"""
from __future__ import annotations

import argparse
import hashlib
import json
from importlib.metadata import distribution
from importlib.resources import files
from pathlib import Path
import subprocess
import sys


def check() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--notices-only", action="store_true")
    args = parser.parse_args()
    project = Path(__file__).resolve().parents[1]
    dist = distribution("qram-simulator")
    entries = list(dist.files or ())
    notices = [entry for entry in entries if ".dist-info/licenses/" in str(entry)]
    if not notices:
        raise ValueError("installed wheel has no third-party notices")
    hashes = {hashlib.sha256(Path(dist.locate_file(entry)).read_bytes()).hexdigest()
              for entry in notices}
    provenance = json.loads((project/"ThirdParty/notice-provenance.json").read_text())
    for record in provenance["files"]:
        if record["path"].endswith("argparse.h"):
            continue  # Source attribution is reproduced in THIRD_PARTY_NOTICES.
        if record["sha256"] not in hashes:
            raise ValueError("installed wheel omitted/changed notice: "+record["path"])
    for name in ("THIRD_PARTY_NOTICES.md", "ThirdParty/notice-provenance.json"):
        if hashlib.sha256((project/name).read_bytes()).hexdigest() not in hashes:
            raise ValueError("installed wheel omitted/changed provenance: "+name)
    import qram_simulator

    installed = Path(qram_simulator.__file__).resolve()
    if not installed.is_relative_to(Path(dist.locate_file("")).resolve()):
        raise ValueError("import resolved outside the installed distribution")
    for name in ("demo.py", "demo_server.py", "demo.html",
                 "word_memory.py", "word_backend.py"):
        if files("qram_simulator").joinpath(name).read_bytes() != (
                project/"bindings/python/qram_simulator"/name).read_bytes():
            raise ValueError("installed wheel omitted/changed product asset: "+name)
    command = next((entry for entry in dist.entry_points
                    if entry.group == "console_scripts" and entry.name == "qram-simulator"), None)
    if command is None or command.value != "qram_simulator.demo:main":
        raise ValueError("installed standalone CLI entry point is missing")
    command.load()  # Import the actual installed native extension and CLI.
    print(json.dumps({"version": dist.version, "installed_package": str(installed),
                      "notice_files": len(notices),
                      "bundled_openmp": [str(entry) for entry in entries
                                         if "libgomp" in str(entry) and ".so" in str(entry)]}),
          flush=True)
    if not args.notices_only:
        subprocess.run([sys.executable, "-m", "pytest", "-q",
                        str(project/"bindings/python/test/test_demo.py"),
                        str(project/"bindings/python/test/test_logical_fidelity.py")], check=True)


if __name__ == "__main__":
    check()
