"""Abstract base class for PySparQ composite operators."""

from __future__ import annotations

from abc import ABC, abstractmethod

from pysparq.operators.condition_mixin import ControllableOperatorMixin


class ControllableOperator(ControllableOperatorMixin, ABC):
    """ABC requiring __call__ and dag implementations.

    Subclasses must implement __call__(self, state) -> None.
    dag() raises NotImplementedError by default.
    """

    @abstractmethod
    def __call__(self, state) -> None:
        """Apply the operator to *state*."""
        raise NotImplementedError

    def dag(self, state) -> None:
        """Apply the adjoint of the operator to *state*."""
        raise NotImplementedError("dag not implemented for this operator")
