"""Assemble a pinned, driver-free CUDA toolkit in a new development directory.

Usage: python tools/bootstrap_cuda.py --output build-cuda/toolkit
Downloads only official NVIDIA redistributables, checks sizes and SHA256,
and preserves component licenses. Does not install drivers or system packages.
"""

import argparse
import hashlib
import json
import os
import platform
import shutil
import subprocess
import tarfile
import urllib.request
from pathlib import Path

BASE = "https://developer.download.nvidia.com/compute/cuda/redist/"


def merge_tree(source, target):
    """Merge component directories, rejecting conflicting content."""
    target.mkdir(parents=True, exist_ok=True)
    for path in sorted(source.iterdir()):
        if path.name == "LICENSE":
            continue
        destination = target/path.name
        if path.is_symlink():
            link = os.readlink(path)
            if destination.is_symlink() and os.readlink(destination) == link:
                continue
            if destination.exists() or destination.is_symlink():
                raise ValueError("conflicting toolkit link: "+str(destination))
            destination.symlink_to(link)
        elif path.is_dir():
            merge_tree(path, destination)
        elif destination.exists():
            if destination.read_bytes() != path.read_bytes():
                raise ValueError("conflicting toolkit file: "+str(destination))
        else:
            shutil.copy2(path, destination)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--release", choices=("12.8.1", "13.3.0"), default="13.3.0")
    args = parser.parse_args()
    if platform.system() != "Linux" or platform.machine() != "x86_64":
        raise ValueError("this bootstrap supports Linux x86_64 only")
    pin = json.loads(Path(__file__).with_name(f"cuda-redist-{args.release}-linux-x86_64.json").read_text())
    root = args.output.resolve()
    root.mkdir(parents=True, exist_ok=False)
    archives, extracted, prefix = root/"archives", root/"components", root/"prefix"
    archives.mkdir()
    extracted.mkdir()
    prefix.mkdir()
    for name, component in pin["components"].items():
        relative = component["relative_path"]
        if relative.startswith("/") or ".." in Path(relative).parts:
            raise ValueError("invalid component path")
        archive = archives/Path(relative).name
        with urllib.request.urlopen(BASE+relative, timeout=60) as response, archive.open("wb") as stream:
            shutil.copyfileobj(response, stream)
        if archive.stat().st_size != int(component["size"]) or hashlib.sha256(archive.read_bytes()).hexdigest() != component["sha256"]:
            raise ValueError("component size or SHA256 mismatch: "+name)
        directory = extracted/name
        directory.mkdir()
        with tarfile.open(archive) as tar:
            tar.extractall(directory, filter="data")
        roots = list(directory.iterdir())
        if len(roots) != 1 or not roots[0].is_dir():
            raise ValueError("unexpected component archive layout: "+name)
        license_path = prefix/"licenses"/name
        license_path.mkdir(parents=True)
        for source in roots[0].glob("LICENSE*"):
            shutil.copy2(source, license_path/source.name)
        merge_tree(roots[0], prefix)
        print("verified and assembled: "+name, flush=True)
    if (prefix/"lib").is_dir() and not (prefix/"lib64").exists():
        (prefix/"lib64").symlink_to("lib", target_is_directory=True)
    version = subprocess.check_output([str(prefix/"bin/nvcc"), "--version"], text=True)
    (root/"provenance.json").write_text(json.dumps({**pin,"nvcc_output":version}, indent=2)+"\n")
    print(version)
    print(prefix)


if __name__ == "__main__":
    main()
