# The Qubit-QRAM Error Propagation Mechanism: A Theoretical Walkthrough with Single-Error Injection Verification

> This document is the mechanism appendix of `qubit_qram_pruning.md`. It answers one question: **why the bad-branch criterion of the qubit encoding cannot reuse the qutrit's subtree containment, and must instead "climb from the left child to the parent node"** (i.e., the current logic of `TimeStep::get_bad_range_qubit`). All conclusions take the code semantics as normative and are verified by an n=3 all-address single-error injection experiment.

## 1. Microscopic Dynamics (Reconstructed from the Code)

Take the noise-free schedule with n=3, k=1 as an example (`TimeStep::generate_step`; for k>3 the window lengthens but the structure is unchanged). The excitation trajectory of a normal query with address i=0b011 (from a measured dump):

| Step | Operation | Excitation location (node:slot) |
|---|---|---|
| 1,2 | ACopy[0] Swap[0] | a0=0 loaded into the root pointer (value 0 → **invisible**) |
| 3,4 | ACopy[1] cSwap[0] | a1 moves down through the root data slot to node1.data |
| 5 | ACopy[2] Swap[1] | a1 is swapped internally into node1.addr (the path pointer is in place) |
| 6,7 | cSwap[0] CopyIn[0]+cSwap[1] | a2 moves down; the bus bit is swapped into the root data slot and descends layer by layer with the cswaps |
| 8–9 | cSwap Swap[2] cSwap[1] | a2 is swapped into the leaf addr (the memory-cell select bit); the bus arrives at the leaf data slot |
| 10 | FetchData | CZ phase fetch |
| 11–16 | cSwap mirror + CopyOut | the bus returns along the same path; the address excitations withdraw layer by layer |
| 17–19 | ACopy_out | the tree is cleared |

Structural key points (specific to the qubit encoding):

1. **There is no idle level**: in `State::cswap` (`qram_branch_qubit.cpp:171`), `addr==0 → swap with the left child`. The pointer value 0 is the same as "idle" and indistinguishable from it.
2. **The cswap layers fire repeatedly**: `cSwap[0]` executes at every even step throughout the whole query (s4,6,8,12,14,16 for n=3,k=1), and the site selection of `cswap_layer` (`qram_branch_qubit.cpp:187-193`) includes **"idle parent nodes whose child has an excitation"** — normal operation relies on the inter-layer pipeline to avoid erroneous send-backs, but **stray excitations migrate under the repeated firing**.
3. **The uncompute withdraws only the path excitations**: any flip on an idle node is never cleaned up and becomes a **permanent residue**.

## 2. Fault Taxonomy (Single X Error, Measured by n=3 All-Address Injection)

Damage comes in two layers, corresponding to the two requirements of the pruning predictor (correct output + the shared final configuration Q̃₀):

- **W (wrongbus)**: the output data is wrong — real output damage;
- **dirt (residue)**: the final tree is non-empty — this component's configuration ≠ the common configuration, breaking good-branch sharing.

Measurements (injecting one X per node × slot × time step, with K=3 and mem[i]=i so that wrong fetches become visible):

| Fault location | wrongbus union | Residue of off-path components |
|---|---|---|
| node0 (root) | all | — |
| node1 (the root's left child) | {0,1,2,3} = subtree(1) | {4..7} all stop at **node1.data** (uniform) |
| node2 (the root's right child) | {4..7} = subtree(2) | {0..3} uniform |
| node3 (node1's left child, leaf) | {0,1} = subtree(3) | **{2,3} stop at node3; {4..7} climb to node1** — family divergence! |
| node4 (node1's right child, leaf) | {2,3} = subtree(4) | {0,1,4..7} stop uniformly at node4 |
| node5/6 (node2's children) | their own subtrees | uniform |

**Conclusion 1 (common to both architectures)**: wrongbus ⊆ subtree(v) always holds — direct mis-routing damage was subtree-contained to begin with.

**Conclusion 2 (specific to the qubit encoding)**: an X error leaves permanent residues in **all** superposition components (no one cleans the 0→1 flip of an idle addr). The residue location is governed by the migration rules, and the off-path components therefore split into distinct "configuration families".

## 3. Residue Migration Rules (the Mechanistic Origin of the Rules of Thumb)

The step-by-step migration of residual excitations is driven by two layers of operators:

- **Pull-up**: when `cswap(p)` fires, if `addr(p)` points to the child holding the residue, that child's data slot is swapped with `p.data` → the residue **moves up one level**;
- **Push-down**: when `cswap(v)` of the residue's own node v fires, the data slot is sent to a child according to `addr(v)` (an idle node has addr=0 → **always leftward**).

**The key asymmetry**: an idle node's addr=0 always points to the left child, hence

- **Residue at a left-child position**: it gets pulled up for every component whose "parent is idle" (parent=0 → continue); only when the parent pointer points right (i.e., the component lies in v's sibling subtree) is it not pulled (parent=1 → stop). This is exactly the mechanism behind **"parent=0 keeps propagating, parent=1 stops"** and **"a left child's error propagates to the parent"**.
- **Residue at a right-child position**: an idle parent (=0) swaps the left child's slot, so a right child is never pulled; moreover, a right child being off-path ⇔ the parent pointer pointing left — **the only parent that would pull it is one pointing right, and that is exactly the case where it lies on the path (an already-damaged family)**. Hence a right-child residue is **trapped inside its own subtree**.
- **The root's children (node1/node2)**: the root always holds a0 (it is never idle), so node1's (left child's) residue keeps climbing only for {0..3} (root pointing left = the already-damaged family); {4..7} stop uniformly at node1 → **the code's special case for node1, "mark only the left half", is mechanically correct**, not an arbitrary truncation.

**Configuration families couple to the collapse**: the pruning predictor requires all good branches to share one final configuration (the |Q̃₀⟩ of Theorem 3). Different residue locations = different families; the simulator's final-state sampling (`remove_mismatch_state` in `sample_output`) keeps only the components of the drawn configuration — the families are mutually coupled, and one cannot predict another family as good. Therefore:

> **The qubit bad interval = the envelope of configuration-family divergence**:
> - Right-child fault: the divergence stays within subtree(v) (the sibling family and the farther families have the same residue location — all stop at v) → bad = subtree(v);
> - Left-child fault: the sibling-subtree components (parent points right, no climbing) and the farther components (parent idle, climb to the parent) have different residue locations → the divergence extends to subtree(parent(v)) → bad = subtree(parent(v));
> - Root fault: bad = all.

This is exactly the current logic of `get_bad_range_qubit` (odd node → the parent's interval, even node → its own interval, node1/node2 special-cased), which at n=3 **matches the measured family divergence case by case** (including the reference-branch choice: the family outside bad happens to be uniform, and the reference branch's configuration is that family's configuration).

## 4. The Qutrit Contrast: Why Subtree Containment Suffices There

Running the same experiment on the qutrit circuit (`run_bitflip`, semantics: data slot 0↔1; the addr slot flips only when not W — **the addr of an idle W simply cannot be flipped**):

| Fault | wrongbus | Off-path components |
|---|---|---|
| addr fault (any node) | subtree(v) | **completely clean** (W cannot be flipped) |
| data fault (same for left/right child) | ⊆ subtree(v) | **uniform residue, frozen in place at v** (the `case W: do nothing` guard of `QRAMState::cswap` + `cswap_layer` traversing only non-zero nodes, so the residue cannot migrate) |

A qutrit's residue **cannot migrate**: all off-path components share one and the same configuration ("all zeros + the residue frozen at v"), and the family divergence **never escapes subtree(v)** — subtree containment is both the damage criterion and the predictor-safety criterion. This is precisely the qubit trap that PR APPL missed when it treated the criterion as encoding-independent: **the criterion's true requirement is not "the damage is confined to the subtree" (which also holds for qubits) but "the off-path components keep the same final configuration" (which fails for qubits because residues migrate)**.

## 5. Impact on the Pruning Algorithm

1. **The damping channel is exempt**: the K₁ jump happens only at excitations (|1⟩), and an idle node has no excitation available to jump → no off-path residue is produced; the mis-routing / stranding damage caused by damping satisfies ⊆ subtree(v). **Under pure damping noise, the qubit case can still use the pure subtree criterion** (the applicability of the H–K₀–H data-bus closed form in this document is unaffected).
2. **The depolarizing channel**: its X/Y components create migrating residues off the path → the family-divergence criterion of this appendix (i.e., the current `get_bad_range_qubit`) must be used.
3. **The bad-fraction scaling changes**: for qubits, a left-child fault marks the parent's interval (doubling the mass), raising the expected bad-branch mass from the qutrit's O(n²p) (each node carries only its own subtree mass) to a version of O(n²p) with a larger constant; in the worst case (near the root), a single fault already marks half or all of the addresses. The pruning benefit for the depolarizing channel drops accordingly, but the complexity class is unchanged.
4. **A deep-tree risk to be verified (unverified for n>3)**: multi-level climbing along idle chains (the left child's left child's left child...) may, in deeper trees, let the divergence escape subtree(parent(v)); the current single-level climb is a candidate scenario for under-approximation. Mechanically, multi-level climbing requires consecutive ancestors that are idle and pointing left, corresponding to components whose addresses have an all-0 high-order prefix — most of which have already fallen into the damaged family, which is why no escape was observed in the n=3 measurements. An injection-scan regression of the same kind is recommended for n≥4 when implementing pruning.

## 6. Verification Method (Reproducible)

Probe: a single-file C++ program (linked against the repository's static library) that injects a single BitFlip per (node × slot × time step), takes all addresses as input, and reports the wrongbus / residue location per address; `QTRACE=1` dumps the tree state step by step. The qubit and qutrit circuits are compared under the same framework. The data correspond one-to-one to the tables in this document.
