# check if pybind11-stubgen is installed
try:
    import pybind11_stubgen
except ImportError:
    print("pybind11-stubgen is not installed, please install it by running 'pip install pybind11-stubgen'")
    exit()

import subprocess

subprocess.run(["pybind11-stubgen", "pysparq", "-o", "pyqsparse"])
# subprocess.run(["pybind11-stubgen", "PyQSparseAlgorithmBlockEncodingTridiagonal", "-o", "pyqsparse/algorithm/block_encoding/tridiagonal"])
