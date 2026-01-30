from pyqsparse import *
from math import pi
import numpy as np

def test_simple_program():
    System.add_register("q0", StateStorageType.General, 4)
    System.add_register("q1", StateStorageType.General, 4)
    System.add_register("q2", StateStorageType.General, 4)

    state = SparseState()

    Hadamard_Int_Full("q0").apply(state)
    # Hadamard_Int_Full("q1").apply(state)
    Zgate_Int("q0", 2).apply(state)
    Hadamard_Int_Full("q0").apply(state)   
    StatePrint(disp=StatePrintDisplay.Detail).apply(state)


if __name__ == '__main__':
    test_simple_program()
