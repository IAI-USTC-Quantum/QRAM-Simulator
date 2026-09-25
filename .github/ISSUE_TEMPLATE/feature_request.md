---
name: Feature Request
about: Suggest a new feature for the project
title: '[Feature] '
labels: enhancement
assignees: ''
---

## Feature Description

<!-- Describe clearly and concisely the feature you would like added -->

## Use Cases

<!-- Describe the scenarios in which this feature would be used and what problem it solves -->

### Background

[Describe the background information]

### Target Users

[Which users does this feature primarily serve]

### Usage Example

```cpp
// Desired C++ API usage example
QRAMSimulator sim(10);  // 10 qubits
sim.load(address, data);
auto result = sim.execute();
```

Or Python:

```python
# Desired Python API usage example
import pyqsparse

sim = pyqsparse.QRAMSimulator(10)
sim.load(address, data)
result = sim.execute()
```

## Desired API/Interface

<!-- If applicable, describe your desired interface design -->

### Proposed Interface

```cpp
// Example class/function declaration
class NewFeature {
public:
    NewFeature(int param);
    Result process(const Input& input);
};
```

### Parameter Description

| Parameter | Type | Description |
|------|------|------|
| `param` | `int` | Parameter description |
| `input` | `Input` | Input description |

## Alternative Solutions

<!-- Describe any alternative solutions you have considered -->

### Option 1: [option name]

[Description]

**Pros:**
- ...

**Cons:**
- ...

### Option 2: [option name]

[Description]

**Pros:**
- ...

**Cons:**
- ...

## Additional Context

<!-- Any other relevant information, such as reference documentation or related projects -->

- Related literature: [link]
- Reference implementation: [link]
- Related issue: #123
