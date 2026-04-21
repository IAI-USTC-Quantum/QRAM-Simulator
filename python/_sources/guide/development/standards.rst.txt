代码规范
========

C++ 规范
--------

- 使用 ``clang-format -i`` 格式化
- 4 空格缩进，120 列限制
- 遵循 LLVM 代码风格
- 添加 ``COMPOSITE_OPERATION`` 宏支持 dagger 操作

命名规范
--------

+----------------------+----------------------------------------+
| 类型                 | 规范                    | 示例          |
+======================+========================+==============+
| 类名                 | UpperCamelCase        | ``GroverOperator`` |
+----------------------+------------------------+--------------+
| 函数/方法            | snake_case            | ``add_register``   |
+----------------------+------------------------+--------------+
| 寄存器名             | 简洁有意义            | ``"addr"``, ``"data"`` |
+----------------------+------------------------+--------------+
| 命名空间             | lowercase             | ``qram_simulator`` |
+----------------------+------------------------+--------------+

性能考虑
--------

- 优先使用稀疏态表示
- 及时清理临时寄存器：``RemoveRegister(temp_reg)(state)``
- 使用 ``profiler`` 追踪性能瓶颈

性能优化示例：

.. code-block:: cpp

   // 及时清理不再使用的寄存器
   auto temp_reg = AddRegister("temp", UnsignedInteger, size)(state);
   // ... 使用 temp_reg ...
   RemoveRegister(temp_reg)(state);  // 释放内存

   // 使用 profiler 定位瓶颈
   {
       profiler _("MySlowOperation");
       ComplexOperation(params)(state);
   }

注释规范
--------

仅在以下情况添加注释：

- 约束条件不明显时
- 存在隐藏的不变量时
- 针对特定 bug 的 workaround

.. code-block:: cpp

   // 不要写：
   // 将 a 加到 b
   Add_UInt_UInt("a", "b")(state);

   // 应该写：
   // Workaround for known issue #123
   Add_UInt_UInt("a", "b")(state);
