"""Check actual wheel notices, embedded GUI assets and the standalone entry point.

Usage: python tools/check_wheel_contents.py path/to/qram_simulator-*.whl
This checks the built artifact, not just the packaging configuration.
"""
from __future__ import annotations

import argparse
import hashlib
import json
from pathlib import Path
from zipfile import ZipFile


def check(wheel: Path) -> None:
    root = Path(__file__).resolve().parents[1]
    provenance = json.loads((root/"ThirdParty/notice-provenance.json").read_text())
    with ZipFile(wheel) as archive:
        notices = {name: archive.read(name) for name in archive.namelist()
                   if ".dist-info/licenses/" in name and not name.endswith("/")}
        hashes = {hashlib.sha256(value).hexdigest() for value in notices.values()}
        for record in provenance["files"]:
            if record["path"].endswith("argparse.h"):
                continue  # source attribution is summarized in THIRD_PARTY_NOTICES
            if record["sha256"] not in hashes:
                raise ValueError("wheel omitted/changed notice: "+record["path"])
        for filename in ("THIRD_PARTY_NOTICES.md", "ThirdParty/notice-provenance.json"):
            digest = hashlib.sha256((root/filename).read_bytes()).hexdigest()
            if digest not in hashes:
                raise ValueError("wheel omitted/changed attribution: "+filename)
        for filename in ("demo.py", "demo_server.py", "demo.html"):
            actual = archive.read("qram_simulator/"+filename)
            expected = (root/"bindings/python/qram_simulator"/filename).read_bytes()
            if actual != expected:
                raise ValueError("wheel omitted/changed GUI asset: "+filename)
        entrypoints = [name for name in archive.namelist()
                       if name.endswith(".dist-info/entry_points.txt")]
        if len(entrypoints) != 1 or b"qram-simulator = qram_simulator.demo:main" not in archive.read(entrypoints[0]):
            raise ValueError("standalone entry point is missing")
    print(f"PASS {wheel.name}: native product assets and {len(notices)} notice files")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("wheel", type=Path)
    check(parser.parse_args().wheel)
