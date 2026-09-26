# Branch Pruning and Fast Simulation for the Qubit-Architecture QRAM: The H–K₀–H Predictability Theory

**Damping update:** the qubit sampler now uses whole-tree joint Kraus sampling and normalized conditional trajectories. Historical nonzero-damping numbers and per-candidate sampling descriptions must be regenerated/replaced; see [joint damping](joint_damping.md).

> Theoretical design document (pre-implementation). Companion to the main publication: Yun-Jie Wang et al., *Efficient Simulation of Quantum Random Access Memory* (arXiv:2503.13832, Phys. Rev. Applied **25**, 044069). This document closes a gap left open by that work: **branch prediction and pruning for the qubit-encoded (standard) bucket-brigade QRAM under amplitude damping noise**.

## Abstract

The sparse-state QRAM simulator (this repository) achieves fast noisy simulation of the qutrit-encoded bucket-brigade QRAM: the noise history is pre-sampled into a "branch tree"; good branches, whose routing paths pass through no noise site, are not actually evolved — their final-state amplitudes are predicted directly by analytic formulas — and only the bad branches, whose good/bad classification is predetermined by the subtree-containment criterion, need explicit evolution. The trick is simple in the qutrit scheme because the data bus travels in the computational basis, on which the no-jump Kraus operator $K_0$ is diagonal, so the entire damping effect on a good branch reduces to a countable scalar decay factor. In the qubit encoding, however, the data bits must traverse the whole tree in the $\{|+\rangle,|-\rangle\}$ basis (the CZ phase-encoding fetch protocol requires a Hadamard first), $K_0=\mathrm{diag}(1,a)$ is non-diagonal in the X basis, and the scalar prediction fails. This document proves that this $H\to K_0^d\to H$ structure is still **predictable in closed form**: the state acted on by the second H gate is exactly of the form $(|0\rangle+(-1)^{b\oplus m}a^d|1\rangle)/\sqrt{2}$, giving output amplitudes $(1\pm a^d)/2$; $k$ data bits yield $2^k$ closed-form components that depend only on the Hamming weight of the error pattern. Building on this, we give a complete pruning algorithm for the qubit QRAM that preserves the same complexity scaling as the qutrit scheme, $\mathcal{O}(d+p\cdot\mathrm{poly}(n))$, and we clarify its error structure: the first-order effect on a good branch is mere norm loss (naturally absorbed into the Monte Carlo trajectory weights), the true within-branch error is second-order coherent leakage, and all first-order errors are concentrated on the explicitly evolved bad branches.

---

## 1. Introduction

A quantum random access memory (QRAM) reads out data at superposed addresses with query depth $\mathcal{O}(\log N)$, making it a core component of algorithms such as quantum database search, quantum state preparation, and quantum machine learning; the price is a binary routing tree requiring $\mathcal{O}(N)$ physical qubits. The sparse-state simulator of Wang et al. [1] enables efficient classical simulation of QRAM based on two observations: (i) the bucket-brigade circuit consists entirely of non-branching reversible gates, so the tree-configuration evolution of each input basis component is deterministic and unique; (ii) the noisy evolution can be expanded over Monte Carlo trajectories, where each noise site affects only those branches whose routing path passes through it (subtree containment), while the final states of all other good branches can be predicted analytically instead of being evolved. The simulation complexity drops from the exponential cost of the full density matrix to $\mathcal{O}(d+p\cdot\mathrm{poly}(n))$ ($d$ is the number of input superposition branches, $p$ the per-qubit noise rate, and $n$ the number of address bits).

That work and its implementation take the **qutrit encoding** as the flagship scheme: routing nodes use the three levels $\{|W\rangle,|L\rangle,|R\rangle\}$, the idle state $|W\rangle$ is a fixed point of the damping channel's no-jump operator $K_0$, and data transfer happens in the computational basis. For the physically more common **qubit encoding** (two two-level bits per routing node, address bit $|0\rangle\equiv L$ and $|1\rangle\equiv R$, with no idle state), fast-simulation support has remained incomplete: the repository's qubit good-branch multiplier implements only the scalar counting of the address-routing part (`TimeStep::_get_multiplier_impl_qubit`, with the wiring code commented out; see `QRAM/include/qram_branch_qubit.h:213-225`), while the data-bus part — the subject of this document — lacked a theoretical formula.

The essential difference between the qubit and qutrit schemes lies in how data is fetched [2]: the qutrit scheme uses a classically controlled CNOT to flip the data bit in the computational basis, whereas the qubit scheme uses **CZ phase encoding** — the classical memory bit acts as the control and applies $Z$ to the data bit passing through its leaf. To make "an active node's $|0\rangle$" distinguishable from "an inactive node", a Hadamard must be applied before the data bus enters the tree, rotating $\{|0\rangle,|1\rangle\}$ into the $\{|+\rangle,|-\rangle\}$ basis, and once the fetched data returns, another H rotates back to the original basis. Each data bit therefore shuttles through the entire tree as an X-basis superposition between the two H gates, continuously subject to the $K_0$ action of amplitude damping — this is the **$H\to K_0^d\to H$ structure** analyzed in this document.

Contributions of this document:

1. Prove that the $H\to K_0^d\to H$ structure of a qubit-QRAM good branch has a closed-form solution (Lemma 1), and give the $2^k$-component Hamming-weight formula for $k$ data bits (Theorem 2);
2. Give the complete final-state prediction theorem for good branches (Theorem 3), extending the v2 proposition of Wang et al. that "all reliable branches share the same final state" [1, Eq. (6)] to the qubit encoding;
3. Analyze the error structure: the deviation of a good branch's output from the ideal output splits into first-order norm loss and second-order coherent leakage, of which the former is not an error but the probability weight of the jump trajectories;
4. Present the complete pruning-based fast simulation algorithm with a complexity analysis, and list the gaps in the current implementation against this repository.

## 2. Background and Notation

### 2.1 The $(n,k)$-QRAM and the Time-Slice Schedule

An $(n,k)$-QRAM implements the transformation
$$
\sum_{i,j}\alpha_{i,j}|i\rangle_A|j\rangle_D\;\longrightarrow\;\sum_{i,j}\alpha_{i,j}|i\rangle_A|j\oplus d_i\rangle_D,
$$
where $n$ is the number of address bits ($N=2^n$), $k$ the number of data bits, and $d_i\in\{0,1\}^k$ the classical memory content [1,2].

A bucket-brigade query pipelines over a full binary tree of depth $n$: address bits are injected one by one at the root and drill down to establish the routing path; data-bus bits are swapped in one by one (CopyIn), travel down the path, return along the same path after interacting with the memory at the leaf, and are swapped out (CopyOut); finally all operations are executed in reverse order to uncompute. The simulator organizes the circuit into $T=6n+2k$ time slices (`TimeStep::full_step`, `QRAM/src/time_step.cpp:118-121`), with the key scheduling points (`time_step.cpp:133-203`):

| Event | Time slice of data bit $i$ |
|---|---|
| Address bit $t$ injected (ACopy) | $2t+1$ |
| CopyIn$_i$ (data bit $i$ swapped into the root) | $\tau_{\mathrm{in}}(i)=2n+2i+1$ |
| FetchData$_i$ (CZ fetch at the leaf) | $\tau_{\mathrm{fetch}}(i)=3n+2i+1$ |
| CopyOut$_i$ (data bit $i$ swapped out) | $\tau_{\mathrm{out}}(i)=4n+2i+1$ |

The schedule is mirror-symmetric about its midpoint: $\mathrm{out}(t)=T-t$. Note that $\tau_{\mathrm{out}}(i)-\tau_{\mathrm{in}}(i)=2n$ is independent of $i$, and that $\tau_{\mathrm{fetch}}(i)$ is exactly the midpoint of the window. In the qubit architecture, the two Hadamards are attached just before CopyIn$_0$ and just after CopyOut$_{k-1}$ (`QRAM/include/qram_circuit_qubit.h:1039-1048`), acting on the entire data-bus register.

### 2.2 Noise Model and Monte Carlo Trajectory Expansion

After the gate operations of each time slice, noise is applied to every physical qubit inside the currently entangled subtree independently with the corresponding probability (`TimeStep::noise_one_step`, `time_step.cpp:341-385`):

- **Amplitude damping** (strength $\gamma$): Kraus operators
$$
K_0=|0\rangle\langle0|+\sqrt{1-\gamma}\,|1\rangle\langle1|=\mathrm{diag}(1,a),\qquad K_1=\sqrt{\gamma}\,|0\rangle\langle1|,
$$
where $a\equiv\sqrt{1-\gamma}$ denotes the per-step decay factor. In the simulation this is split into two parts: $K_0$ acts deterministically as the **default operator** (`Damp_Common`: at every step, each excitation amplitude currently in $|1\rangle$ is multiplied by $a$, `qram_branch_qubit.h:84-87`); the jump operator $K_1$ is handled only at pre-sampled noise sites as a **quasi-measurement** (`Damp_Full`: compute the jump probability from the current unnormalized state, roll the dice, and on a hit project and reset the bit across all branches, `qram_circuit_qubit.h:468-558`).
- **Depolarizing** (strength $\varepsilon$): a random Pauli $X/Z/Y$, applied explicitly at the sampled noise sites.

One trajectory = one pre-sampled noise history (the positions and kinds of the noise sites) + the everywhere-action of the default $K_0$. The state is always kept as an (unnormalized) sparse pure state, normalized only at quasi-measurements and at final-state sampling [1, SI §III]. Averaging over many trajectories restores the density-matrix-level observables.

### 2.3 The Three Predictability Pillars of the Qutrit Scheme

The pruning algorithm of Wang et al. rests on three pillars [1]:

**(P1) Unique tree-configuration theorem.** The circuit consists entirely of non-branching gates such as CSWAP/SWAP/CNOT/CZ (they only permute computational basis states and create no superpositions), so for every address $|i\rangle$ there is a unique tree-configuration orbit $|\Psi_i(t)\rangle$; the noise-free evolution can be replayed classically without solving any equations.

**(P2) Diagonality of the default operator.** $K_0$ is diagonal in the nodes' computational basis, so along a no-jump trajectory a good branch's basis-state orbit is exactly the noise-free orbit, and the entire damping effect is multiplication by a **countable scalar decay factor**: the amplitude factor $a^{c}$, where $c$ is the branch's "number of excitations × time steps" count, given analytically by the schedule table without any simulation. The relative weight between different good branches is $a^{\Delta c}$ (implementation: `TimeStep::get_multiplier_qutrit`, `QRAM/include/time_step.h:209-234`).

**(P3) Subtree-containment criterion.** A fault at node $(l,p)$ affects only the address interval corresponding to its leaf subtree,
$$
i\in\big[2^{\,n-l}p,\;2^{\,n-l}(p+1)-1\big],
$$
so, given the noise history, the good/bad branch sets can be determined analytically **before any evolution** (implementation: `get_bad_range_qutrit` + the interval union `ContinuousRange`, `time_step.cpp:267-339`). The expected bad-branch fraction is $\mathcal{O}(n^2p)$ [1, v1 Eq. (6)].

**A caveat on the encoding dependence of (P3) (see the companion document `qubit_error_propagation.md` for details).** The true foundation of this criterion for qutrits is not that "the output damage is confined to the subtree" (that also holds for qubits), but that "**the off-path components keep the same final tree configuration**": a qutrit's fault residue is frozen in place at the faulty node by the $|W\rangle$ guard (the `case W: do nothing` of `QRAMState::cswap`), all off-path components share one and the same configuration, and good-branch prediction is therefore valid. The qubit encoding lacks this guard, so the residues of X-type faults **migrate and get pulled up** along the "idle = points left" ancestor chain, diverging the configuration families over a wider range — the bad interval must instead use the left-child climb rule (§3, §5.1).

In the qutrit scheme, data transfer happens in the computational basis and requires no H gates; the two H attachment points degenerate into pure merge operations (`run_hadamard` in `qram_branch_qutrit.cpp:547-573` only calls `try_merge`), so (P2) directly covers the data bus: good branches do not split, and the final state is $|i\rangle|j\oplus d_i\rangle|Q_0\rangle$ times a scalar factor.

## 3. The Gaps in the Qubit Scheme

In the qubit encoding, pillar (P1) holds verbatim (the non-branching gate set is independent of the level encoding). There are two gaps — the data-bus part of (P2), and the configuration identity of (P3):

- **(P3′) Configuration-family divergence (X-type faults)**: the output damage (wrongbus) itself still satisfies ⊆ subtree(v) (verified by single-error injection), but an X flip leaves permanent residues in **all** superposition components (no one cleans idle nodes), and these residues migrate upward via the "idle = points left" ancestor pull: a residue at a left-child position climbs one level for every component whose parent is idle, while a residue at a right-child position is trapped inside its own subtree. The off-path components therefore split into different final-configuration families, and the envelope of the family divergence is **bad = the left child takes subtree(parent(v)), the right child takes subtree(v)** (i.e. the current `get_bad_range_qubit`; the root and the root's children are special-cased). Damping jumps (K₁) create no off-path residues (an idle node has no excitation available to jump), so **the pure damping channel can still use the pure subtree criterion**. For the mechanism derivation and the exhaustive n=3 injection verification, see `qubit_error_propagation.md`.
- **(P2) Address/routing part**: routing bits in the state $|1\rangle$ are likewise decayed step by step by $K_0$; the exposure window is given by the schedule table and is still a countable scalar (already implemented in the repository as `_get_multiplier_impl_qubit`, `time_step.cpp:462-475`). Nothing new is needed here.
- **(P2) Data-bus part**: data bit $i$ is swapped in at step $\tau_{\mathrm{in}}(i)=2n+2i+1$ and swapped out at step $\tau_{\mathrm{out}}(i)=4n+2i+1$, and in between it lives inside the tree in the form $(|0\rangle\pm|1\rangle)/\sqrt{2}$. $K_0$ is non-diagonal in the X basis:
$$
K_0|+\rangle=\tfrac{1}{\sqrt2}\big(|0\rangle+a|1\rangle\big),\qquad K_0|-\rangle=\tfrac{1}{\sqrt2}\big(|0\rangle-a|1\rangle\big).
$$
Different data components within a branch decay out of sync, and the two H gates no longer cancel each other — **the output of the second H gate is no longer a definite computational basis state**. This is exactly why the qutrit scalar-multiplier formula fails, and why a naive implementation must let the branch evolve for real (splitting at every H, counting step by step, and merging at the end).

## 4. Main Theoretical Results

### 4.1 The H–K₀–H Lemma

**Lemma 1 (single-bit closed form).** Let $a=\sqrt{1-\gamma}$, $d\in\mathbb{N}$, $m\in\{0,1\}$, and define
$$
N_{m,d}\;=\;H\,\underbrace{Z^{m}\,\mathrm{diag}(1,a^{d})}_{\text{diagonal, and merges with }K_0\text{ powers}}\,H .
$$
Then for any input bit $b\in\{0,1\}$, writing $b_{\mathrm{out}}=b\oplus m$:
$$
N_{m,d}\,|b\rangle\;=\;\alpha_d\,|b_{\mathrm{out}}\rangle+\beta_d\,|\overline{b_{\mathrm{out}}}\rangle,\qquad
\alpha_d=\frac{1+a^{d}}{2},\;\;\beta_d=\frac{1-a^{d}}{2}.
$$

**Proof.** $H|b\rangle=\big(|0\rangle+(-1)^{b}|1\rangle\big)/\sqrt2$. Between the two H gates, the only actions on this bit are: (i) $d$ applications of $K_0=\mathrm{diag}(1,a)$ (routing SWAPs merely relocate the bit and never change its state; see below); (ii) the $Z^m$ contributed by the CZ fetch at the leaf. Both are diagonal in the computational basis and commute with each other, so they merge into $Z^m\mathrm{diag}(1,a^d)=\mathrm{diag}\big(1,(-1)^m a^d\big)$. The resulting state is $\big(|0\rangle+(-1)^{b\oplus m}a^d|1\rangle\big)/\sqrt2$ — **this is the precise meaning of "when the second H gate acts, it is as if $|0\rangle+a^d|1\rangle$ passes through one more H"** (the sign is determined jointly by the input bit and the memory bit). Passing through $H$ once more:
$$
\frac{1}{2}\Big[\big(1+(-1)^{b\oplus m}a^{d}\big)|0\rangle+\big(1-(-1)^{b\oplus m}a^{d}\big)|1\rangle\Big],
$$
and collecting terms according to $b_{\mathrm{out}}=b\oplus m$ gives the claim. $\square$

**Remark 1 (norm-conservation consistency).** $\alpha_d^2+\beta_d^2=\frac{1+a^{2d}}{2}$ is exactly the direct evaluation of the **no-jump probability** $\frac12(\langle0|+\langle1|)K_0^{d\dagger}K_0^{d}(|0\rangle+|1\rangle)/1$ of an X-basis state passing through $d$ layers of the damping channel. The missing norm $\frac{1-a^{2d}}{2}$ is precisely the probability weight carried by the jump ($K_1$) trajectories — evolved explicitly by the bad branches in this algorithm. Under the unnormalized framework, the two parts of the ledger balance exactly.

**Remark 2 (the correct and the erroneous component).** $\alpha_d$ is the amplitude of a "correct fetch", and $\beta_d$ is the amplitude coherently leaked into the wrong value. When $a=1$ (noise-free), $\beta_d=0$ and the ideal QRAM output $H^2=I$ is recovered.

### 4.2 The Multi-Bit Product Structure

**Theorem 2 (the $H$–$K_0$–$H$ formula for $k$ data bits).** On a good branch $(i,j)$ (address $i$, bus input $j\in\{0,1\}^k$), suppose data bit $i$ resides inside the tree for $d_i=\tau_{\mathrm{out}}(i)-\tau_{\mathrm{in}}(i)=2n$ damping layers, with memory bit $m_i=(d_i)_{\text{the }i\text{-th bit}}$ (where the notations clash, context decides; below the memory content is written $d$, with its $t$-th bit $d^{(t)}$). Then the unnormalized state of the data register after the second H is
$$
\bigotimes_{t=0}^{k-1}N_{d^{(t)},\,2n}\,|j_t\rangle
\;=\;\sum_{c\in\{0,1\}^k}A(c)\,|c\rangle,
$$
where the amplitude of an output component $c$ depends only on the **Hamming weight** $w=|e|$ of the error pattern $e=c\oplus b_{\mathrm{out}}$ ($b_{\mathrm{out}}=j\oplus d$ being the ideal output):
$$
\boxed{\;A(c)\;=\;\frac{1}{2^{k}}\,(1+a^{2n})^{\,k-w}\,(1-a^{2n})^{\,w}\;=\;\alpha_{2n}^{\,k-w}\,\beta_{2n}^{\,w}\;}
$$

**Proof.** The routing of different data bits is guided by the same address path and pipelined through in different time slices, with no gate coupling between them; each bit's fetch $Z^{d^{(t)}}$ and its $K_0$ exposure are independent. The total diagonal operator between the two H gates is therefore a bit-wise tensor product, and the total map is $\bigotimes_t N_{d^{(t)},d_t}$. By the schedule table, $d_t=\tau_{\mathrm{out}}(t)-\tau_{\mathrm{in}}(t)=(4n+2t+1)-(2n+2t+1)=2n$ is the same for all bits (an endpoint-counting convention contributes at most a $\pm1$ correction, identical for all bits, and it is self-calibrated by the reference branch; see §5.2). Applying Lemma 1 to each bit, $c_t=b_{\mathrm{out},t}$ contributes $\alpha_{2n}$ and $c_t=\overline{b_{\mathrm{out},t}}$ contributes $\beta_{2n}$; collecting terms by the number of wrong bits $w$ gives the claim. $\square$

**The contrast with the qutrit counter is worth emphasizing**: in the qutrit version the data window depends on the input/output bit values ($b_{\mathrm{in}},b_{\mathrm{out}}$ determine whether an excitation exists inside the tree on the outbound/return trip; see the data part of `_get_multiplier_impl_qutrit`, `time_step.cpp:510-534`), whereas in the qubit version the data bit **always resides inside the tree in a half-excited state for the full window $2n$**, regardless of the bit value — X-basis transport makes the exposure window trivial, which actually simplifies the counting.

### 4.3 The Scalar Factor of the Address-Routing Part

The excitation exposure of routing bits belongs to pillar (P2), as in the qutrit case. When the $t$-th bit of address $i$ is 1, the corresponding excitation is injected at step $2t+1$ and withdrawn with the uncompute at step $\mathrm{out}(2t+1)=T-(2t+1)$ (the qubit scheme has no idle state, so the path pointers live for the entire query). The full-trajectory exposure count is
$$
c_{\mathrm{addr}}(i)=\sum_{t:\,i_t=1}\big(T-2(2t+1)\big)=\sum_{t:\,i_t=1}(6n+2k-4t-2),
$$
consistent with `_get_multiplier_impl_qubit` (`time_step.cpp:462-475`). The whole-branch amplitude acquires the scalar factor $a^{c_{\mathrm{addr}}(i)}$; the relative probability weight between different good branches is
$$
\frac{p_i}{p_{i_{\mathrm{ref}}}}=(1-\gamma)^{\,c_{\mathrm{addr}}(i)-c_{\mathrm{addr}}(i_{\mathrm{ref}})}\;\equiv\;a^{2\Delta c},
$$
i.e. the `relative_multiplier` of `get_multiplier_qubit` (`time_step.h:186-207`).

Note that the address exposure window in the qubit scheme is the **whole query** (of order $\sim T$), whereas the qutrit scheme activates address excitations in a propagating fashion, exposing each bit for only $\mathcal{O}(t)$ steps (the $[2t+1,3t+2]$ segment and its mirror in `time_step.cpp:490-509`) — this is exactly the protocol-level origin of the qubit scheme's infidelity scaling $\mathcal{O}((n+k)n^2\varepsilon)$ being a factor $n$ worse than the qutrit scheme's $\mathcal{O}((n+k)n\varepsilon)$ [2, §III]; it is unrelated to the simulation algorithm itself, but worth noting in the physical interpretation.

### 4.4 The Good-Branch Final-State Prediction Theorem

**Theorem 3 (closed-form prediction for good branches).** Given a noise history, let the branch $(i,j)$ be a good branch (its routing path passes through no noise site). Then the final state of this branch's no-jump trajectory is
$$
|\Psi_{i,j}^{\mathrm{good}}\rangle
\;=\;a^{\,c_{\mathrm{addr}}(i)}\;|i\rangle_A\otimes\Big[\bigotimes_{t=0}^{k-1}N_{d^{(t)},2n}|j_t\rangle\Big]_D\otimes|\tilde Q_0\rangle_{\mathrm{tree}},
$$
where $|\tilde Q_0\rangle_{\mathrm{tree}}$ is the (unnormalized) final tree state, **identical for all good branches and for the reference branch**.

**Proof.** By (P1), the reversible gates inside the tree merely relocate the unique basis-state orbit; by (P3) there are no noise sites on the path, so the only nontrivial action inside the tree is $K_0$ at every step. Decompose $K_0^{\otimes}$ bit by bit: the routing bits contribute the scalar $a^{c_{\mathrm{addr}}(i)}$ counted as in §4.3; the data bits' contribution is absorbed into the closed form of $N$ as in §4.1–4.2 (each step spent inside the tree contributes one factor $a$, independent of the residing position, because $K_0$ acts uniformly on every node's bits). The final tree-state orbit coincides with the noise-free orbit, i.e. with the address-independent common final state (a definite configuration reached before the uncompute returns everything to all-zeros / all $W$), with only its norm corrected by the scalar. $\square$

**Relation to the proposition of Wang et al.** [1, v2 Eq. (6)] asserts that all reliable branches share the final state $|i\rangle|d_i\rangle|Q_0\rangle$. Theorem 3 is its qubit generalization: the sharing still holds ($|\tilde Q_0\rangle$ is address-independent), but the data register's definite value $|j\oplus d_i\rangle$ is replaced by a **known two-component product state** $\bigotimes_t(\alpha|b_{\mathrm{out},t}\rangle+\beta|\overline{b_{\mathrm{out},t}}\rangle)$. This is the entire difference between the two encodings at the prediction level.

### 4.5 Error Structure: Norm Loss and Coherent Leakage

The final state of Theorem 3 can be expanded in orders of $\gamma$. For a single bit ($d=2n$ layers):

- **Correct amplitude** $\alpha_{2n}=\frac{1+(1-\gamma)^n}{2}=1-\frac{n\gamma}{2}+\mathcal{O}(\gamma^2)$;
- **Leakage amplitude** $\beta_{2n}=\frac{1-(1-\gamma)^n}{2}=\frac{n\gamma}{2}+\mathcal{O}(\gamma^2)$.

Two notions must be distinguished:

1. **Norm loss (first order, not an error).** The single-bit no-jump probability is $\alpha^2+\beta^2=\frac{1+a^{4n}}{2}=1-n\gamma+\mathcal{O}(\gamma^2)$. The lost $\approx n\gamma$ is the probability weight of the jump trajectories, carried by the explicit evolution of the bad branches within the Monte Carlo framework. The shrinking unnormalized norm of a good branch is **not an output error**.
2. **Coherent leakage (second order, a real error).** After conditional normalization within a good branch, the error probability of a single data bit is
$$
\frac{\beta_{2n}^2}{\alpha_{2n}^2+\beta_{2n}^2}=\frac{n^2\gamma^2}{4}+\mathcal{O}(\gamma^3).
$$

**Conclusion: the output error of a good branch is second order in $\gamma$; all first-order errors are concentrated on the bad branches (the jump and depolarizing noise sites).** This explains why "pruning + closed-form prediction" loses no first-order accuracy in the qubit encoding, and it is consistent with the channel-averaged result: the error probability of a single data bit after the full damping channel is $\frac{1-a^{2n}}{2}\approx\frac{n\gamma}{2}$, whose first-order term $=$ jump probability $\times\frac12$ (the output is random after a jump), while the second-order remainder is exactly the good branch's coherent leakage.

This property is good news for error-filtration-type applications [1, §IV; 4]: combined with the implicit postselection on jumps (a good-only trajectory is exactly the output under the "no jump" assumption), the residual error is $\mathcal{O}(n^2\gamma^2)$ per data bit.

### 4.6 The Analytic Contribution of Good Branches to Jump-Probability Sampling

The quasi-measurement probability at a $Damp\_Full$ noise site must sum the occupation weight on the faulty bit over **all** branches (including the good branches that are not evolved). Two observations let the qubit scheme keep the analytic conversion:

1. **The data part is input-independent.** Under X-basis transport, every residing data bit has average occupation probability exactly $1/2$ at every position it passes, independent of the input $j$ and the memory $d$ (the $\pm$ phases do not affect probabilities). Hence the data-part jump probability of the reference good branch holds **exactly** for all good branches, with no per-branch correction.
2. **The address part is converted via the relative multiplier.** Same as the qutrit version: $\mathrm{prob}_{\mathrm{damp}}^{(i)}=\mathrm{prob}_{\mathrm{damp}}^{(\mathrm{ref})}\times\mathrm{relative\_multiplier}_i$ (implementation: `qram_circuit_qubit.h:496-519`).

Therefore, introducing the data-bus closed form of this document does not break the global consistency of the jump sampling.

## 5. The Algorithm

### 5.1 Pseudocode

```
Algorithm 1: noisy fast simulation of the qubit-architecture (n,k)-QRAM (single trajectory)
Input:  sparse input state Σ_j c_{i,j} |i⟩|j⟩; memory table d; noise rates (ε_dep, γ); a = √(1−γ)
Output: the noisy output sparse state (unnormalized), or a final-state sample

1  Noise-history pre-sampling: for t = 1..6n+2k, binomially sample the error count
   according to the active-subtree size 2(2^{L(t)}−1), and sample positions uniformly;
   merge every noise site into the union R of bad address intervals:
   Damp_Full → the node's leaf subtree (damping jumps create no off-path residue);
   the X/Y components of Depolarizing → the configuration-family divergence criterion
   (get_bad_range_qubit: a left child takes subtree(parent(v)), a right child takes
   subtree(v); the root / root's children are special-cased).
2  Branch partition: address i ∈ R ⇒ bad branch; otherwise good branch, with the
   first good branch recorded as the reference branch ref.
3  Real evolution: execute time slice by time slice only for bad branches ∪ {ref}:
   apply H before CopyIn_0 and H after CopyOut_{k−1} (a 2^k-way split within the
   branch + try_merge merges); at the end of every step, Damp_Common (amplitude ×
   a^{#excitations}); at Damp_Full sites, convert the probability per §4.6 and
   sample the projection.
4  Analytic prediction of good branches: for every good branch (i, j):
   a) address factor: count c_addr(i) as in §4.3; relative weight r_i = a^{2[c_addr(i)−c_addr(ref)]};
   b) data factor: b_out = j ⊕ d_i; for every error pattern c with w = 0..k,
      A(c) = α^{k−|c⊕b_out|} β^{|c⊕b_out|}, α=(1+a^{2n})/2, β=(1−a^{2n})/2;
   c) tree final state: copy the tree final state of ref.
5  Output reconstruction: every good branch writes 2^k components |i⟩|c⟩ into the
   output sparse state, with amplitude = c_{i,j} × √(r_i) × A(c) × (the ref tree
   final-state amplitude);
   bad branches are written from their evolution results; merge identical terms and
   drop components with |amplitude|² < ε.
6  Final-state sampling (optional): roulette-wheel select the final state according
   to the unnormalized weights, then normalize.
```

### 5.2 Correctness

- Steps 1–3 are identical to the qutrit version; their correctness follows from (P1), (P3), and the trajectory method [1].
- The correctness of step 4 is Theorem 3; the endpoint convention ($\pm1$) of the window count $d_t=2n$ is identical for all good branches and self-calibrates against the step-by-step `Damp_Common` count in the reference branch's real evolution — the relative weight $r_i$ does not involve this convention, and the absolute factor $(1+a^{2n})/2$ can be calibrated once and for all by comparing the $2^k$ components against the reference branch.
- In step 5, the $2^k$ expansion of a good branch is an exact identity (Theorem 2), not an approximation; compared with the qutrit version's "multiply by $\sqrt{\mathrm{multiplier}}$ + correct the bus output" (`SparQ/src/qram.cpp:100-170`, now in the SparQSim repository), it only adds an expansion loop that distributes weights according to $A(c)$.

### 5.3 Complexity

Let $d$ be the number of input branches and $B$ the number of bad branches (with an expectation of order $\mathbb{E}[B]=\mathcal{O}(n^2p)\,d$; see [1, v1 Eq. (6)]):

| Item | Qutrit version | Qubit version (this document) |
|---|---|---|
| Number of actually evolved branches | $B+1$ | $B+1$ |
| Prediction cost per good branch | $\mathcal{O}(n+k)$ (scalar multiplier) | $\mathcal{O}(n+k)$ (counting) $+\,\mathcal{O}(k\,2^k)$ (writing out the components) |
| Memory per good branch | $\mathcal{O}(n)$ | $\mathcal{O}(n+k)$ (the product state can be stored lazily) |
| Total complexity | $\mathcal{O}(d+p\cdot\mathrm{poly}(n))$ | $\mathcal{O}(d\cdot 2^k+p\cdot\mathrm{poly}(n))$ |

The $2^k$ factor is the intrinsic output width of the qubit encoding (each data bit carries a two-component superposition), not an algorithmic inefficiency: the output data register of a noise-free qubit QRAM is a definite $k$-bit value in the first place, and once noise is present its support naturally widens to $2^k$. For larger $k$, the product structure allows lazy expansion (generate the components on demand at sampling/reconstruction time via the Hamming-weight formula, at an expected $\mathcal{O}(k)$ per component), or truncation by $\beta/\alpha$ to $w\le w_{\max}$ (error $\mathcal{O}((n\gamma)^{w_{\max}+1})$). For the typical $k=1\sim3$ in the experiments, this factor is negligible.

## 6. Comparison with the Qutrit Scheme

| Aspect | Qutrit scheme | Qubit scheme (this document) |
|---|---|---|
| Idle state | $\|W\rangle$ is a fixed point of $K_0$ | None; the routing $\|1\rangle$ is exposed for the whole run |
| Data fetch | CNOT (computational-basis flip) | CZ (phase encoding), requires a basis change by $H$ |
| The two H gates | Degenerate (only try_merge) | Real splitting/merging |
| Data-bit damping exposure | Computational basis, the window depends on $b_{\mathrm{in}},b_{\mathrm{out}}$ | X basis, fixed window $2n$, independent of the bit value |
| Good-branch data output | The definite value $\|j\oplus d_i\rangle$ | The known product state $\bigotimes_t(\alpha\|b_{\mathrm{out},t}\rangle+\beta\|\overline{b_{\mathrm{out},t}}\rangle)$ |
| Error within a good branch | 0 (pure scalar decay) | $\mathcal{O}(n^2\gamma^2)$ coherent leakage per bit |
| Address-excitation exposure | $\mathcal{O}(t)$ steps/bit (propagating) | The whole run, $\sim T$ steps/bit (residing) |
| X-type fault residue | Frozen at the faulty node (W guard), off-path configurations identical | Migrates along idle ancestors, families diverge up to subtree(parent) |
| Prediction formula | The scalar $a^{\Delta c}$ | The scalar $a^{\Delta c}$ × the Hamming-weight formula |
| Bad-interval criterion | Subtree containment (common to both channels) | Damping: subtree containment; depolarizing X: left-child climb (§3) |

## 7. Implementation Gaps and Roadmap in This Repository

Against the current code, landing this algorithm requires:

1. ~~**Wire up qubit noise sampling**~~ (done): `fill_bad_range` now supports the qubit architecture (`time_step.cpp`), and `arch2str` gained the qubit branch.
2. ~~**Full (unpruned) ground truth**~~ (done): `qram_qubit::QRAMCircuit` gained `set_input_uniform`/`set_input_random`, `run_full()` (= evolve all branches), and `sample_and_get_fidelity()`; fixed the probability-conversion index of `BranchGroup::get_prob_damp`, the weight convention of `sample_output_*` (`branch_probs × |amp|²`), and the missing jump reset in `Branch::run_damp_full` (after the projection the bit must be set back to $|0\rangle$), and implemented `Branch::get_fidelity`. Experiment entry point: `Experiment_QRAM_FidelityV2 --architecture qubit` (full-only).
3. ~~**Data-bus closed-form prediction (the core gap)**~~ (done): the good-branch final state is realized by **XOR-mirror materialization** — `data_bus ^= (b_out^{ref} ⊕ b_out^{good})`, amplitude × √relative_multiplier — copying out the $2^k$ components directly from the reference branch (`materialize_good_branches`, `qram_circuit_qubit.h`). The mathematical basis is exactly Theorem 2 (the Hamming-weight formula + the $d_i=2n$ input-independence); after materialization, none of the generic paths (probability/sampling/fidelity) need a good-branch special case.
4. ~~**Good-branch wiring for the qubit version**~~ (done): `sample_output` was rewritten as a walk over all groups after materialization (same order and same weights as full mode, guaranteeing identical collapses under the same seed); `normalization` covers all groups; fixed the **double counting of input weights** in the good-branch conversion (the grouped `get_prob()` already contains the group's input weight, so one must divide out the reference group's weight to get unit norm — the flat qutrit branches do not have this problem; it was introduced during the port).
5. **The data part of the jump-probability conversion**: the qubit `run_damp_full` (`qram_circuit_qubit.h`) already contains the reference-branch conversion framework; just add the data part per §4.6 (input-independent, occupation always $1/2$).
6. ~~**Validation**~~ (done): under 1e-5 depol+damping, n=3..10, 50 seeds per point, `test_qubit_compare` (`--architecture qubit`) shows **full/normal agreement in all cases (400/400)**; 150 trials of strong-noise stress (1e-4~3e-4) also all pass; at n=12, pruning yields a 17× speedup (2565ms→150ms). Remaining: the good-only mode (the qubit counterpart of `QRAMLoadFast`).
7. ~~**Bad-interval criterion confirmed**~~ (done): the "left child climbs to the parent's interval, right child takes its own subtree" logic of `get_bad_range_qubit` was verified by mechanism derivation and n=3 single-error injection to be **the correct envelope of the configuration-family divergence** (the node1 special case is also mechanically correct: the root is never idle); it had previously been mis-documented as a "suspected quirk". For the derivation see `qubit_error_propagation.md`. Two notes: the pure damping channel admits the tighter pure-subtree criterion; and whether "multi-level climbing along idle chains" on deeper trees with n≥4 escapes subtree(parent(v)) is unverified — an n=4/5 injection regression is recommended when implementing pruning.

## 8. Conclusion

We have shown that the good branches of a qubit-encoded bucket-brigade QRAM under amplitude damping noise remain predictable in closed form: the $H\to K_0^d\to H$ structure acts on the $t$-th data bit as the known single-bit map $N|b\rangle=\alpha|b\oplus d^{(t)}\rangle+\beta|\overline{b\oplus d^{(t)}}\rangle$, where $d=2n$ is the fixed exposure window, $\alpha=(1+a^{2n})/2$, and $\beta=(1-a^{2n})/2$; the $k$-bit output amplitudes depend only on the Hamming weight of the error pattern. Combined with the existing address scalar multiplier and subtree-containment pruning, noisy simulation of the qubit QRAM reaches the same complexity scaling as the qutrit version, with only $\mathcal{O}(n^2\gamma^2)$ coherent leakage as the error within a good branch. This provides the complete theoretical foundation for completing the qubit architecture's noisy fast simulation in this simulator, and for supporting downstream studies such as error filtration.

## References

1. Y.-J. Wang, T.-P. Sun, X.-N. Zhuang, X.-F. Xu, H.-Y. Liu, C. Xue, Y.-C. Wu, Z.-Y. Chen, G.-P. Guo, *Efficient Simulation of Quantum Random Access Memory* (v2: *Refined Criteria for QRAM Error Suppression via Efficient Large-Scale QRAM Simulator*), Phys. Rev. Applied **25**, 044069; arXiv:2503.13832.
2. Z.-Y. Chen et al., the $(n,k)$-QRAM parallel query protocol (the bucket-brigade query circuit in both the qubit and qutrit encodings), arXiv:2303.05207.
3. C. T. Hann, G. Lee, S. M. Girvin, L. Jiang, *Resilience of quantum random access memory to generic noise*, PRX Quantum **2**, 020311 (2021).
4. S. Lee et al., *Error filtration for quantum communication/quantum memory* (the error filtration scheme), Phys. Rev. Lett. **131**, 190601 (2023).
5. V. Giovannetti, S. Lloyd, L. Maccone, *Quantum Random Access Memory*, Phys. Rev. Lett. **100**, 160501 (2008).

---

### Appendix: Key Notation

| Notation | Meaning |
|---|---|
| $n,k$ | Number of address bits, number of data bits; $N=2^n$ |
| $T=6n+2k$ | Total number of time slices |
| $\gamma,\ a=\sqrt{1-\gamma}$ | Per-layer damping strength, per-step decay factor |
| $K_0,K_1$ | No-jump / jump Kraus operators of amplitude damping |
| $\tau_{\mathrm{in}}(i),\tau_{\mathrm{out}}(i)$ | Time slices at which data bit $i$ is swapped in/out; window $d_i=2n$ |
| $\alpha_d,\beta_d$ | $(1\pm a^d)/2$, the correct / leakage amplitudes |
| $c_{\mathrm{addr}}(i)$ | Routing-excitation exposure count of address $i$ |
| $b_{\mathrm{out}}$ | $j\oplus d_i$, the ideal output data |
| $w$ | Hamming weight of the error pattern $c\oplus b_{\mathrm{out}}$ |
