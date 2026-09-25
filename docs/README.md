# QRAM-Simulator Documentation

English | [简体中文](README.zh-CN.md)

This directory holds the sources of the **Sphinx site** (published on
[GitHub Pages](https://iai-ustc-quantum.github.io/QRAM-Simulator/)). The site
is **bilingual (English by default, Chinese available)** and carries:

- **User guide** (installation / quickstart / architecture / operator
  constraints / naming conventions) — Markdown (MyST)
- **C++ API reference** — Doxygen header comments ingested into Sphinx via
  **breathe**
- **Python API reference** — `.pyi` stubs generated from the compiled
  pybind11 module by **pybind11-stubgen**, built by **sphinx-autoapi**
- **Papers & experiments** — paper-to-code walkthroughs and reproduction
  guides

The Python bindings (`qram-simulator` PyPI package) live in
`bindings/python/` of this repository; the rich `pysparq` bindings live in
the [SparQSim repository](https://github.com/IAI-USTC-Quantum/SparQSim).

## Directory layout

```
docs/
├── sphinx/
│   ├── Makefile               # make html (builds both trees)
│   ├── requirements.txt       # Sphinx build dependencies
│   └── source/
│       ├── _conf_base.py      # shared Sphinx config (theme, breathe, autoapi)
│       ├── _shared/           # shared static files + templates (language switcher)
│       ├── api_stubs/         # CI-injected .pyi stubs (gitignored)
│       ├── en/                # English tree (default language)
│       │   ├── conf.py, index.rst
│       │   ├── guide/         # install/quickstart/architecture/operators/naming
│       │   ├── api/           # C++ API (cpp.rst, breathe) + Python API (autoapi)
│       │   └── paper/         # paper reproduction & theory docs
│       └── zh/                # Chinese tree (same layout as en/)
└── README.md                  # this file (English; see README.zh-CN.md)
```

(`Doxyfile` sits in the repository root and outputs to `docs/api/` — the
HTML is for direct browsing, the XML feeds breathe; both are gitignored.)

## Local build

```bash
# 1. Doxygen XML (breathe input; requires doxygen on the system)
doxygen Doxyfile

# 2. Python extension and type stubs (autoapi input; only needed for the
#    Python API chapter)
pip install . pybind11-stubgen -r docs/sphinx/requirements.txt
pybind11_stubgen qram_simulator -o docs/sphinx/source/api_stubs

# 3. Sphinx site (both language trees)
sphinx-build docs/sphinx/source/en docs/sphinx/build/html/en
sphinx-build docs/sphinx/source/zh docs/sphinx/build/html/zh
# or: cd docs/sphinx && make html
```

> Without step 1 the C++ API chapter fails to build; without step 2 the
> Python API chapter is skipped automatically (`_conf_base.py` detects
> whether `api_stubs/` exists).

## CI (docs.yml)

push/PR trigger -> doxygen -> pip install . + stubgen -> sphinx-build (zh +
en) -> artifact upload; on push to main the site is deployed to gh-pages
(full replacement; the root URL redirects to the English tree).
