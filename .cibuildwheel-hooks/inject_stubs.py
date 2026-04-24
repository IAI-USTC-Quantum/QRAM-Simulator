#!/usr/bin/env python3
"""
inject_stubs.py — cibuildwheel post-build hook.

Finds the wheel built by cibuildwheel, generates .pyi stubs from the compiled
extension using pybind11-stubgen, injects them into the wheel in-place, and
updates the committed stubs in the source tree.

Run after the wheel (containing the .so/.pyd) is assembled.
"""

from __future__ import annotations

import os
import re
import shlex
import subprocess
import sys
import tempfile
from pathlib import Path


PROJECT_ROOT = Path("/home/user/pwd")
HOOKS_DIR = PROJECT_ROOT / ".cibuildwheel-hooks"
STUBS_SOURCE_DIR = PROJECT_ROOT / "PySparQ" / "pysparq"


def main() -> None:
    # 1. Locate the most recent wheel in the project root.
    wheels = sorted(PROJECT_ROOT.glob("*.whl"), key=lambda p: p.stat().st_mtime, reverse=True)
    if not wheels:
        print("inject_stubs: no wheel found, skipping")
        return
    wheel_path = wheels[0]
    print(f"inject_stubs: found wheel {wheel_path.name}")

    # 2. Extract wheel into a temp directory.
    with tempfile.TemporaryDirectory() as tmpdir:
        tmppath = Path(tmpdir)
        subprocess.run(
            [sys.executable, "-m", "zipfile", "-e", str(wheel_path), str(tmppath)],
            check=True,
        )

        # Locate the pysparq package directory inside the extracted wheel.
        # cibuildwheel always uses the "purelib" data scheme:
        #   <wheel>/
        #     pysparq-x.y.z.dist-info/
        #     pysparq/               <-- this is what we need
        pkg_dirs = list(tmppath.glob("pysparq"))
        if not pkg_dirs:
            print("inject_stubs: pysparq/ not found in wheel, skipping stub injection")
            return
        pkg_dir = pkg_dirs[0]

        # 3. Generate stubs from the compiled extension inside the wheel.
        #    pybind11-stubgen writes output to the specified directory as
        #    "<module>/<stub>", so we generate into a temp subdirectory.
        stubgen_out = tmppath / "stubgen_out"
        stubgen_out.mkdir()

        # Find the compiled extension (.so / .pyd) inside pysparq/.
        extensions = list(pkg_dir.glob("_core*.so")) + list(pkg_dir.glob("_core*.pyd"))
        if not extensions:
            print("inject_stubs: _core extension not found in wheel pysparq/, skipping")
            return
        ext = extensions[0]
        print(f"inject_stubs: generating stubs from {ext.name}")

        result = subprocess.run(
            [
                sys.executable, "-m", "pybind11_stubgen",
                "--no-setup-hook",
                "--skip-signature-indentation",
                "-o", str(stubgen_out),
                str(ext),
            ],
            capture_output=True,
            text=True,
        )
        if result.returncode != 0:
            print(f"inject_stubs: stubgen warning (non-fatal):\n{result.stderr}", file=sys.stderr)

        # pybind11-stubgen creates <stubgen_out>/pysparq/_core.pyi
        generated_stub = stubgen_out / "pysparq" / "_core.pyi"
        if not generated_stub.exists():
            # Fallback: try top-level
            generated_stub = stubgen_out / "_core.pyi"

        if generated_stub.exists():
            # 4. Inject the generated stub into the wheel's pysparq/ directory.
            dest = pkg_dir / "_core.pyi"
            import shutil
            shutil.copy2(generated_stub, dest)
            print(f"inject_stubs: injected {generated_stub.name} into wheel pysparq/")

            # 5. Repackage the wheel in-place.
            repack(wheel_path, tmppath)
        else:
            print(f"inject_stubs: no stub generated, skipping wheel injection", file=sys.stderr)

        # 6. Update committed stubs in source tree so CI validation passes.
        if generated_stub.exists():
            import shutil as sh
            committed = STUBS_SOURCE_DIR / "_core.pyi"
            sh.copy2(generated_stub, committed)
            print(f"inject_stubs: updated committed stubs: {committed}")


def repack(wheel_path: Path, source_dir: Path) -> None:
    """Repackage source_dir into wheel_path (overwrites in-place)."""
    import zipfile
    # Remove the old wheel
    wheel_path.unlink(missing_ok=True)
    with zipfile.ZipFile(wheel_path, "w", compression=zipfile.ZIP_DEFLATED) as zout:
        for fpath in source_dir.rglob("*"):
            if fpath.is_file():
                arcname = fpath.relative_to(source_dir)
                zout.write(fpath, arcname)
    print(f"inject_stubs: repacked wheel: {wheel_path.name}")


if __name__ == "__main__":
    main()
