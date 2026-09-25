# Configuration file for the Sphinx documentation builder.
#
# For the full list of built-in configuration values, see:
# https://www.sphinx-doc.org/en/master/usage/configuration.html

from __future__ import annotations

from pathlib import Path

# -- Project information -----------------------------------------------------

project = "QRAM-Simulator"
copyright = "2021-2026, IAI-USTC-Quantum"
author = "IAI-USTC-Quantum"
release = "0.2.0"

# 中文文档设置
language = "zh_CN"

# -- General configuration ---------------------------------------------------

extensions = [
    "sphinx.ext.autodoc",
    "sphinx.ext.autosummary",
    "sphinx.ext.napoleon",
    "sphinx.ext.viewcode",
    "sphinx.ext.intersphinx",
    "sphinx.ext.todo",
    "myst_parser",
    "sphinx_copybutton",
    "breathe",
]

templates_path = ["_templates"]
exclude_patterns = [
    "_build",
    "Thumbs.db",
    ".DS_Store",
    "api_stubs",  # pybind11-stubgen 生成的 .pyi（CI 注入，见 docs.yml）
]

# -- Options for HTML output -------------------------------------------------

html_theme = "furo"
html_static_path = ["_static"]
html_title = "QRAM-Simulator 文档"

# Theme options for furo（与 SparQSim 文档同风格）
html_theme_options = {
    "light_css_variables": {
        "color-brand-primary": "#2563eb",
        "color-brand-content": "#1d4ed8",
    },
    "dark_css_variables": {
        "color-brand-primary": "#3b82f6",
        "color-brand-content": "#60a5fa",
    },
    "sidebar_hide_name": False,
    "navigation_with_keys": True,
    "source_repository": "https://github.com/IAI-USTC-Quantum/QRAM-Simulator/",
    "source_branch": "main",
    "source_directory": "docs/sphinx/source/",
}

# -- Options for MyST parser -------------------------------------------------
# https://myst-parser.readthedocs.io/en/latest/configuration.html

source_suffix = {
    ".rst": "restructuredtext",
    ".md": "markdown",
}

myst_enable_extensions = [
    "colon_fence",
    "deflist",
    "dollarmath",
    "html_admonition",
    "html_image",
    "replacements",
    "smartquotes",
    "tasklist",
]

# 为 h1-h3 生成 GitHub 风格锚点，使迁移文档内的目录链接
# （如 [宽度与截断约定](#宽度与截断约定)）可解析
myst_heading_anchors = 3

# -- Options for Breathe (Doxygen C++ API) -----------------------------------
# Doxygen XML 由 `doxygen Doxyfile` 先行生成（docs.yml 中完成）；
# 本地构建：仓库根目录运行 doxygen 后再 make html。

_doxygen_xml = Path(__file__).resolve().parents[2] / "api" / "xml"

breathe_projects = {"QRAM-Simulator": str(_doxygen_xml)}
breathe_default_project = "QRAM-Simulator"
breathe_domain_by_extension = {
    "h": "cpp",
}

# -- Options for AutoAPI (Python 绑定 API) -----------------------------------
# .pyi 桩由 pybind11-stubgen 在 CI 中生成到 source/api_stubs/（docstring
# 来自编译模块，全部为中文）。本地未生成时跳过 Python API 章节。

_api_stubs = Path(__file__).resolve().parent / "api_stubs"
if _api_stubs.exists():
    extensions.append("autoapi.extension")

    autoapi_type = "python"
    autoapi_dirs = [str(_api_stubs)]
    autoapi_root = "api/python"
    autoapi_file_patterns = ["*.pyi", "*.py"]
    autoapi_generate_api_docs = True
    autoapi_add_toctree_entry = True
    autoapi_options = [
        "members",
        "undoc-members",
        "show-inheritance",
        "show-module-summary",
        "imported-members",
    ]
    autoapi_python_class_content = "both"
    autoapi_member_order = "groupwise"
    autoapi_keep_private_level = 0

# -- Options for Intersphinx -------------------------------------------------

intersphinx_mapping = {
    "python": ("https://docs.python.org/3/", None),
}
