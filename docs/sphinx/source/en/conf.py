# Configuration file for the Sphinx documentation builder (English tree).
#
# All shared settings live in ../_conf_base.py; only the language-specific
# bits are set here.

import os
import sys

sys.path.insert(0, os.path.abspath(".."))

from _conf_base import *  # noqa: F401,F403,E402

# English documentation settings
language = "en"
html_title = "QRAM-Simulator Documentation"
