# Third-party notices

QRAM-Simulator's project license remains Apache-2.0. Vendored components
retain their own notices and license terms; these files accompany source
distributions and binary wheels in the distribution's `licenses/` directory.

- **{fmt}**: Copyright (c)2012-present Victor Zverovich and {fmt} contributors.
  The existing full MIT-style notice is `ThirdParty/fmt/LICENSE`. The local
  `format.h` adaptation supports formatting `cuda::std::complex` values.
- **Eigen3.4.0**: copyright holders are identified in the original headers.
  Core headers retain their MPL-2.0 notices. The original release's complete
  `COPYING.*` set is included under `ThirdParty/eigen-3.4.0/`, including its
  explanation of file-specific and alternative terms. Source is available
  in this repository and from https://gitlab.com/libeigen/eigen at commit
  `3147391d946bb4b6c68edd901f2add6ac1f31f8c`.
- **argparse**: Author Jesse Laning. The vendored header declares
  "Apache2.0 with LLVM Exception or GPLv3". Distribution uses its declared
  Apache-2.0-with-LLVM-exception option; its header notice is unchanged.
  The canonical Apache text and LLVM exception are included in
  `ThirdParty/argparse/LICENSE.Apache-2.0-with-LLVM-exception.txt`.
  LLVM is the source of that canonical license text, not a bundled dependency.
- **pybind11 3.0.2**: the isolated Python build pins these binding headers.
  Its full BSD-3-Clause copyright and terms accompany wheels as
  `ThirdParty/pybind11/LICENSE`.
- **GCC runtime**: Linux OpenMP builds link to libgomp; manylinux wheel repair
  can bundle this runtime. Its original copyright declaration, GPLv3 text and
  GCC Runtime Library Exception 3.1 accompany distributions under
  `ThirdParty/gcc-runtime/`. These are runtime notices, not a change to this
  project's license. CUDA SDK libraries are not bundled in the CPU wheel.

`ThirdParty/notice-provenance.json` records the exact source commits,
download URLs, SHA256 values and the extraction of canonical license text.
It does not replace the original header attributions. Developer-only
googletest keeps its own license in the source checkout and is excluded
from the Python wheel and trimmed source distribution.
