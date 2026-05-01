"""Condition-setting mixin for PySparQ composite operators.

This module provides the canonical condition-setting interface used by all
PySparQ composite operators.  Subclasses inherit the four condition methods
instead of reimplementing them.

Design: the mixin stores its state in ``__condition_regs`` / ``__condition_bits``
(name-mangled to avoid conflicts with subclasses that write to ``_condition_regs``
/ ``_condition_bits`` directly in ``__call__`` bodies), and exposes the public
names via properties.  Subclass ``__call__`` bodies that read or assign
``self._condition_regs`` / ``self._condition_bits`` interact with the property
storage, which in turn reads/writes the mixin's mangled names.
"""

from __future__ import annotations

try:
    from typing import Self  # Python 3.11+
except ImportError:
    from typing_extensions import Self  # Python 3.10


class ControllableOperatorMixin:
    """Mixin providing the standard condition-setting interface.

    Provides:
        conditioned_by_nonzeros(cond)   -> self
        conditioned_by_all_ones(cond)   -> self
        conditioned_by_bit(reg, pos)    -> self
        clear_conditions()             -> self

    Subclasses MUST implement __call__(self, state) -> None.
    Subclasses SHOULD implement dag(self, state) -> None
    if the operator is not self-adjoint.
    """

    # Mixin storage uses double-underscore → name-mangled, so subclass
    # _condition_regs assignments in __call__ bodies do NOT conflict.
    __condition_regs: list
    __condition_bits: list

    def __init__(
        self,
        *,
        condition_regs: list | None = None,
        condition_bits: list | None = None,
    ) -> None:
        object.__setattr__(self, '__condition_regs', condition_regs or [])
        object.__setattr__(self, '__condition_bits', condition_bits or [])

    @property
    def condition_regs(self) -> list:
        return object.__getattribute__(self, '__condition_regs')

    @condition_regs.setter
    def condition_regs(self, value: list) -> None:
        object.__setattr__(self, '__condition_regs', value)

    @property
    def condition_bits(self) -> list:
        return object.__getattribute__(self, '__condition_bits')

    @condition_bits.setter
    def condition_bits(self, value: list) -> None:
        object.__setattr__(self, '__condition_bits', value)

    @property
    def _condition_regs(self) -> list:
        return self.condition_regs

    @_condition_regs.setter
    def _condition_regs(self, value: list) -> None:
        self.condition_regs = value

    @property
    def _condition_bits(self) -> list:
        return self.condition_bits

    @_condition_bits.setter
    def _condition_bits(self, value: list) -> None:
        self.condition_bits = value

    def conditioned_by_nonzeros(self, cond: str | int | list) -> Self:
        object.__setattr__(
            self,
            'condition_regs',
            cond if isinstance(cond, list) else [cond],
        )
        return self

    def conditioned_by_all_ones(self, cond: str | int | list) -> Self:
        object.__setattr__(
            self,
            'condition_regs',
            [cond] if isinstance(cond, (str, int)) else list(cond),
        )
        return self

    def conditioned_by_bit(self, reg: str | int, pos: int) -> Self:
        object.__setattr__(self, 'condition_bits', [(reg, pos)])
        return self

    def clear_conditions(self) -> Self:
        object.__setattr__(self, 'condition_regs', [])
        object.__setattr__(self, 'condition_bits', [])
        return self
