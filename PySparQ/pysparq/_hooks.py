"""
scikit-build-core wheel.post hook: generate pybind11 stubs into the wheel.

pybind11-stubgen analyzes the compiled .so / .pyd directly — no Python interpreter needed.
The generated .pyi files land inside the wheel's pysparq/ directory and are therefore
automatically included in the wheel with no extra configuration.
"""

from pathlib import Path
import subprocess
import sysconfig


def wheel_post_package(wheel_dir: Path, **kwargs) -> None:
    pysparq_dir = wheel_dir / "pysparq"

    # Locate the compiled pybind11 extension module(s) inside the wheel.
    # scikit-build-core installs them under pysparq/.
    extensions = list(pysparq_dir.glob("_core*.so")) + list(
        pysparq_dir.glob("_core*.pyd")
    )
    if not extensions:
        return  # nothing to generate stubs from

    ext = extensions[0]
    stubgen_cmd = [
        sys.executable,    # use the same Python that invoked the hook
        "-m", "pybind11_stubgen",
        "--no-setup-hook",
        "--skip-signature-indentation",
        "-o", str(pysparq_dir),
        str(ext),
    ]
    subprocess.run(stubgen_cmd, check=False)  # non-fatal: warn but don't fail the build
