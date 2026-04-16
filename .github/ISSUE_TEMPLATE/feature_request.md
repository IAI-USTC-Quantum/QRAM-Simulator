---
name: 功能请求
about: 为项目提出新功能建议
title: '[Feature] '
labels: enhancement
assignees: ''
---

## 功能描述

<!-- 清晰简洁地描述你希望添加的功能 -->

## 使用场景

<!-- 描述这个功能将在什么场景下使用，解决什么问题 -->

### 背景

[描述背景信息]

### 目标用户

[这个功能主要服务于哪些用户]

### 使用示例

```cpp
// 期望的 C++ API 使用示例
QRAMSimulator sim(10);  // 10 量子比特
sim.load(address, data);
auto result = sim.execute();
```

或 Python：

```python
# 期望的 Python API 使用示例
import pyqsparse

sim = pyqsparse.QRAMSimulator(10)
sim.load(address, data)
result = sim.execute()
```

## 期望的 API/接口

<!-- 如果适用，描述你期望的接口设计 -->

### 提议的接口

```cpp
// 类/函数声明示例
class NewFeature {
public:
    NewFeature(int param);
    Result process(const Input& input);
};
```

### 参数说明

| 参数 | 类型 | 说明 |
|------|------|------|
| `param` | `int` | 参数描述 |
| `input` | `Input` | 输入描述 |

## 替代方案

<!-- 描述你考虑过的其他替代方案 -->

### 方案 1: [方案名称]

[描述]

**优点:**
- ...

**缺点:**
- ...

### 方案 2: [方案名称]

[描述]

**优点:**
- ...

**缺点:**
- ...

## 额外上下文

<!-- 任何其他相关信息，如参考文档、相关项目等 -->

- 相关文献: [链接]
- 参考实现: [链接]
- 相关 Issue: #123
