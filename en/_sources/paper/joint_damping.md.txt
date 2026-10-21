# Exact joint sampling of qubit amplitude damping

The qubit engine resolves each damping layer with **one joint auxiliary-mask draw**, followed by one sparse Kraus update and one global normalization. It keeps candidate pre-sampling and branch prediction. It does not calculate a population or rescale the state separately at every physical tree site.

This page supersedes older descriptions of independent `Damp_Full` draws and raw trajectory norms as survival probabilities. The implementation applies to the **qubit** architecture; the qutrit engine still has its historical sampler. A qutrit “faithful mirror” regression is not a certificate of the standard damping channel.

## Channel and noise placement

For each tree qubit, with $0\leq\gamma<1$,

$$K_0=|0\rangle\langle0|+\sqrt{1-\gamma}|1\rangle\langle1|,
\qquad K_1=\sqrt\gamma|0\rangle\langle1|.$$

Every scheduled damping slice applies this channel to **all** $M=2(2^n-1)$ tree qubits. Address and external bus registers are noiseless. Pauli faults retain the scheduler's active-front placement. The two placements are intentionally distinct and are matched by the circuit reference. Within a slice, sampled Pauli operators execute before the contiguous joint damping layer. Candidate generation retains its RNG order, but candidate operation records are placed after the Pauli operators. Intervening gates or mismatched strengths inside a joint damping layer are rejected.

Whole-tree damping matches the existing `Damp_Common` operator and analytic good-branch exposure counts. Previously, K1 candidates were drawn only on the active front, while K0 attenuated the whole tree. A non-active excited qubit therefore lacked its jump partner. The discrepancy can occur even at early root-copy steps with an empty active front. Enlarging the damping candidate domain is part of the physical-channel correction, not just a change of random-number convention.

## Joint auxiliary construction

At the beginning of the damping layer, write the normalized coherent state as $|\psi\rangle=\sum_x c_x|x\rangle$. Let $X(x)$ be its excited tree positions.

1. Pre-sample a candidate set $C$: include each tree site independently with probability $\gamma$. The existing binomial-count plus uniform-subset construction has this distribution.
2. Draw one auxiliary computational-basis configuration with probability $|c_x|^2$.
3. Set the actual jump set to $J=C\cap X(x)$.
4. Apply $K_J=\prod_{q\in J}K_{1,q}\prod_{q\notin J}K_{0,q}$ to the **original coherent state**, then normalize it.

The auxiliary configuration is not a physical measurement of the simulated state. Replacing the state by $|x\rangle$ would destroy coherence and implement a different channel.

For a fixed jump set,

$$\Pr(J)=\sum_{x:J\subseteq X(x)}|c_x|^2\gamma^{|J|}(1-\gamma)^{|X(x)|-|J|}
=\langle\psi|K_J^\dagger K_J|\psi\rangle.$$

Consequently, averaging the normalized conditional states with these probabilities gives exactly $\sum_JK_J\rho K_J^\dagger$. This is a discrete finite-$\gamma$ identity, not a first-order time-step approximation. Local damping channels commute; their sequential conditional probabilities need not be equal.

The C++ implementation samples only the restriction $C\cap X$, by aggregating a `std::map<vector<size_t>, double>` of masks. This is the exact marginal of step 2 and avoids constructing irrelevant parts of the auxiliary state. Canonically sorted mask keys align the roulette intervals in pruned and full execution. There is one uniform draw per **nonempty candidate layer**, even when only one mask has nonzero weight; empty candidate layers consume no auxiliary draw.

## Sparse update and branch prediction

For a stored component with excitation set $X$:

- discard it if $J\not\subseteq X$;
- otherwise erase $J$ and multiply its amplitude by $(\sqrt{1-\gamma})^{|X|-|J|}$.

The common factor $(\sqrt\gamma)^{|J|}$ is omitted as an overall conditional-state scale. The whole represented trajectory is normalized once after the layer. Its stored norm is not an unconditional survival probability or an importance weight.

`damping_mask_weights` includes all evolved groups and all implicit good groups. The latter use the reference group's **joint distribution on the candidate mask**, multiplied by their input weight and analytic address attenuation. This requires the stronger shared-reference invariant: all good groups have the same candidate-mask distribution per unit surviving weight, not merely the same single-site populations. Candidate bad-range exclusion and the shared-tree construction are the basis of this invariant; the regression suite checks its resulting trajectories across multi-candidate histories.

`get_multiplier_qubit(gamma, step)` counts attenuation before this layer; `step + 1` is used after applying K0 and before the common normalization. Only materialized groups are rescaled; implicit groups inherit the same scale through their reference. An empty reference still implies empty represented good groups at reconstruction.

The input-column convention remains explicit: separate `branches` under one address carry orthogonal input-column labels for the probability bookkeeping. `branch_probs` does **not** encode coherent interference between different bus-input columns. The physical QRAM circuit comparisons use the data-loading input, one bus-input column per address. Arbitrary complex superpositions inside a single branch are covered by the local-channel tests. An arbitrary coherent multi-column API needs separate coherent aggregation and is not claimed here.

## What was wrong with the previous sampler?

The old code used $\Pr(C_q)=\gamma$ and $\Pr(\mathrm{jump}\mid C_q)=p_1$. It **did not multiply gamma twice**. Statements that the second C++ draw was $\gamma p_1$ confused the unconditional jump probability with the conditional acceptance probability.

The actual defect was the joint law. Every candidate received a fresh roulette draw; a fired jump updated the state, but rejection left it unchanged until the final `Damp_Common`. Correlated no-jump information was absent from subsequent population calculations. For $q_A=\langle n_A\rangle$, $q_B=\langle n_B\rangle$, and $q_{AB}=\langle n_An_B\rangle$,

$$q_{B\mid A0}=\frac{q_B-\gamma q_{AB}}{1-\gamma q_A}.$$

For $(|00\rangle+|11\rangle)/\sqrt2$, the standard probabilities of zero, one and two jumps are $1-\gamma+\gamma^2/2$, $\gamma-\gamma^2$, and $\gamma^2/2$. The old candidate-only sequence gives $1-\gamma+\gamma^2/4$, $\gamma-3\gamma^2/4$, and $\gamma^2/2$. At gamma 0.2, the single-jump probability was 0.17 instead of 0.16. Normalizing every finished trajectory does not repair that incorrect sampling law.

Two other issues must not be hidden by a matching internal baseline:

- A sampled Kraus branch's raw norm cannot be averaged again as if it were an independent physical survival probability. Normalized-trajectory averages and weighted, unnormalized estimators require different bookkeeping.
- The old state-dependent “faithful” density-matrix mirror was not a fixed linear CPTP channel. Its agreement with the engine, and pruned/full equality, could not certify standard amplitude damping. The qubit channel reference now uses fixed standard Kraus matrices.

## Complexity and operational changes

Let $S$ be the number of explicitly stored components, $h$ their maximum sparse excitation count, $c=|C|$, and $G$ the number of predicted groups. Mask extraction visits excited positions and performs binary searches in sorted candidates; it does not scan $M$ sites or perform $M$ population reductions. Map aggregation additionally depends on the number and lengths of distinct masks. The projection/attenuation is a sparse component pass, followed by one represented-norm reduction and one scale update. Predicted weights cost a sum over $G$ and analytic exposure evaluations.

This does not prove a polynomial overall QRAM cost: candidate generation/storage still has expected size $M\gamma$ per layer; state support, bus-width expansion, mask aggregation and prediction bookkeeping also count. No end-to-end speedup claim is inferred from the local-channel identity.

Candidate sets, RNG consumption, normalized trajectory amplitudes and seed-to-history mappings change. Old nonzero-damping data and timing figures must be regenerated. Fixed seed equivalence is required between the **new** full and pruned engines, not between new and retired samplers. The public `run_damp_full` remains a singleton-candidate projection primitive for source compatibility; loops of independent calls do not implement the joint layer. Normal execution uses `run_damping_layer`.

## Reproducible validation

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DCACHED_REGISTER_SIZE=8
cmake --build build -j
ctest --test-dir build --output-on-failure
./build/bin/CorrectnessTest
./build/bin/ExecutabilityTest
./build/bin/Experiment_QRAM_QubitPaperExactTest --perf-shard 0 1
```

- `QubitJointDampingChannel`: 80 fully enumerated cases, 1–4 qubits, gamma in {0, 1e-5, 0.2, 0.7, 0.95}, ground/excited/GHZ/random-complex inputs. Every density-matrix entry is compared with an independently implemented dense sequential Kraus channel. Also 60,000 Bell-state roulette trials and per-layer norm checks. The exact comparison catches coherence loss as well as incorrect jump probabilities.
- `QubitPaperExactness` and `QubitPaperExactnessChannels`: complete weighted complex components before and after tree measurement, address probabilities, fidelity, RNG states, candidate histories and actual joint jump labels. Fixed histories exercise reference death/survival and circuit reuse.
- `verify_noisy_simulation`: a separately represented 9-qubit gate circuit ($n=2,k=1$) shares the QRAM schedule but not the state or channel backend. Fixed-history matrix replay and standard density-matrix evolution are tested. The qubit ensemble uses 2,000 shots per pure-depolarizing, pure-damping and mixed-noise setting, including gamma 0.2. Both traces must be one; full density-matrix Frobenius error is assessed against its estimated Monte Carlo RMS error, alongside output TVD and fidelity. The remaining qutrit mirror tests are explicitly labeled legacy.

Observed initial checks: local maximum density error $4.45\times10^{-16}$; Bell single-jump frequency 0.15875 versus exact 0.16; circuit output TVD 0.0017–0.0133 and full-density Frobenius error 0.0089–0.0165, compatible with Monte Carlo errors. These are finite regression results, not a proof of every shared-reference implementation invariant.

The external uniqc/QuTiP exporter/driver also passes pure-damping and mixed-noise checks (2,000 shots each, six fixed-history replays). New exports declare `joint_auxiliary_whole_tree_v1`; the driver selects standard Kraus channels and normalizes layer replays.

Final pruned/full validation: all 155 main cases and 120 channel cases passed. The main battery fired 178 jumps; maximum weighted component difference was 6.67e-16 and maximum fidelity difference was 1.34e-15. For an external pure-damping export, explicitly pass `--depolarizing 0` (the CLI default is nonzero).
