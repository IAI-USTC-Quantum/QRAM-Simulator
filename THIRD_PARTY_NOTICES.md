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

`ThirdParty/notice-provenance.json` records the exact source commits,
download URLs, SHA256 values and the extraction of canonical license text.
It does not replace the original header attributions. Developer-only
googletest keeps its own license in the source checkout and is excluded
from the Python wheel and trimmed source distribution.
