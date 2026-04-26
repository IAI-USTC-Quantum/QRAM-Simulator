# Configuration file for the Sphinx documentation builder.
#
# For the full list of built-in configuration values, see the documentation:
# https://www.sphinx-doc.org/en/master/usage/configuration.html

from __future__ import annotations

import os
import sys
from pathlib import Path

# Add PySparQ to path for autodoc
project_root = Path(__file__).parent.parent.parent.parent
sys.path.insert(0, str(project_root / "PySparQ"))

# -- Project information -----------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#project-information

project = "PySparQ"
copyright = "2021-2026, IAI-USTC-Quantum"
author = "IAI-USTC-Quantum"
release = "0.0.1"

# 中文文档设置
language = "zh_CN"

# -- General configuration ---------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#general-configuration

extensions = [
    "sphinx.ext.autodoc",
    "sphinx.ext.autosummary",
    "sphinx.ext.napoleon",
    "sphinx.ext.viewcode",
    "sphinx.ext.intersphinx",
    "sphinx.ext.todo",
    "myst_parser",
    "sphinx_copybutton",
    "autoapi.extension",
    "nbsphinx",
]

templates_path = ["_templates"]
exclude_patterns = ["_build", "Thumbs.db", ".DS_Store"]

# -- Options for HTML output -------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#options-for-html-output

html_theme = "furo"
html_static_path = ["_static"]
html_title = "PySparQ 文档"

# Theme options for furo
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

# -- Options for AutoAPI -----------------------------------------------------
# https://sphinx-autoapi.readthedocs.io/

autoapi_type = "python"
autoapi_dirs = ["../../../PySparQ/pysparq"]
autoapi_root = "autoapi"
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
autoapi_ignore = []

# -- Options for Intersphinx -------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/extensions/intersphinx.html

intersphinx_mapping = {
    "python": ("https://docs.python.org/3/", None),
    "numpy": ("https://numpy.org/doc/stable/", None),
}

# -- Options for Napoleon ----------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/extensions/napoleon.html

napoleon_google_docstring = True
napoleon_numpy_docstring = False
napoleon_include_init_with_doc = True
napoleon_include_private_with_doc = False
napoleon_include_special_with_doc = True
napoleon_use_admonition_for_examples = False
napoleon_use_admonition_for_notes = False
napoleon_use_admonition_for_references = False
napoleon_use_keyword = True
napoleon_use_param = True
napoleon_use_rtype = True
napoleon_preprocess_types = False
napoleon_type_aliases = None
napoleon_attr_annotations = True

# -- Options for Todo --------------------------------------------------------

todo_include_todos = True

# -- Copy button options -----------------------------------------------------

copybutton_prompt_text = r">>> |\.\.\. |\$ |In \[\d*\]: | {2,5}\.\.\.: | {5,8}: "
copybutton_prompt_is_regexp = True
copybutton_line_continuation_character = "\\"
copybutton_here_doc_delimiter = "EOT"

# -- nbsphinx options --------------------------------------------------------
# https://nbsphinx.readthedocs.io/

nbsphinx_execute = "always"  # Re-execute on every build to capture fresh output
nbsphinx_allow_errors = True   # Continue building docs even if a notebook cell fails
nbsphinx_timeout = 60
# Note: nbsphinx prolog/epilog templates use env.docname instead of docname
# in newer versions. We omit prolog for simplicity.
