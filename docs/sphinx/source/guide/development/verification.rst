测试验证
========

验证清单
--------

- [ ] 态矢量验证：与已知结果对比振幅
- [ ] 保真度测试：使用参考实现对比
- [ ] 噪声敏感性分析
- [ ] 性能基准测试

态矢量验证
----------

对比实现的态矢量与理论值或参考实现：

.. code-block:: cpp

   // 提取振幅
   double success_prob = 0;
   std::vector<complex_t> full_amplitudes(full_size);
   for (auto& s : state) {
       full_amplitudes[s.get(addr_reg).value] = s.amplitude;
   }

   // 对比目标态振幅
   for (auto target_position : target_positions) {
       success_prob += abs_sqr(full_amplitudes[target_position]);
   }

   fmt::print("Success probability: {}\n", success_prob);

保真度测试
----------

.. code-block:: cpp

   double fidelity = get_fidelity();  // 使用内置保真度计算
   fmt::print("Fidelity: {}\n", fidelity);

噪声敏感性分析
--------------

.. code-block:: cpp

   struct GroverTestArguments {
       double depolarizing = 0.0;
       double damping = 0.0;

       std::map<OperationType, double> generate_noise() const {
           std::map<OperationType, double> noise;
           if (depolarizing > 0.0)
               noise[OperationType::Depolarizing] = depolarizing;
           if (damping > 0.0)
               noise[OperationType::Damping] = damping;
           return noise;
       }
   };

性能基准测试
------------

.. code-block:: cpp

   // 使用 profiler 追踪性能
   {
       profiler _("MyAlgorithm");
       MyAlgorithm(params)(state);
   }

   fmt::print("{}\n", profiler::get_all_profiles_v2());

命令行参数解析模板
------------------

.. code-block:: cpp

   inline MyArgs parse_arguments(int argc, const char** argv) {
       argparse::ArgumentParser parser("MyAlgorithm", "Description");
       parser.add_argument()
           .names({ "-q", "--qubit" })
           .description("qubit number")
           .required(false);

       parser.enable_help();
       auto err = parser.parse(argc, argv);
       if (err || parser.exists("help")) {
           parser.print_help();
           return {};
       }

       MyArgs args;
       if (parser.exists("qubit"))
           args.qubit = parser.get<int>("qubit");

       return args;
   }
