#pragma once

#include "error_handler.h"
#include "random_engine.h"

namespace qram_simulator {

	/// 时间单位换算常量（以纳秒为 1）
	constexpr double nanosec = 1;
	constexpr double microsec = nanosec / 1e3;  ///< 微秒
	constexpr double millisec = microsec / 1e3; ///< 毫秒
	constexpr double sec = millisec / 1e3;      ///< 秒
	constexpr double minute = sec / 60;         ///< 分
	constexpr double hour = minute / 60;        ///< 时
	constexpr double day = hour / 24;           ///< 天

	/// 当前日期时间的字符串（完整格式）
	std::string _datetime();

	/// 当前日期时间的字符串（简明格式，用于文件名）
	std::string _datetime_simple();

	/**
	 * @brief 简单计时器：构造时记录起点，get 返回经过时间。
	 */
	struct timer {
		/// 计时起点（steady_clock）
		std::chrono::time_point<std::chrono::steady_clock> startpoint;
		timer() {
			startpoint = std::chrono::steady_clock::now();
		}
		/// 距起点的经过时间（按 unit 换算，默认单位为纳秒）
		inline double get(double unit) const {
			std::chrono::nanoseconds m = std::chrono::steady_clock::now() - startpoint;
			return std::chrono::duration_cast<std::chrono::nanoseconds>(m).count() * unit;
		}
	};

	/**
	 * @brief 文件日志单例。
	 *
	 * 首次 instance() 时自动创建带时间戳的日志文件（log-*.txt），
	 * 提供 info / error 两级写入、栈式计时器与 flush。
	 */
	struct Logger {
		std::ofstream out;
		bool on = true;
		std::vector<timer> timers;
		Logger() { }

		/// 取 Logger 单例（首次调用自动新建日志文件并开启写入）
	static Logger& instance()
		{
			static Logger static_logger;
			static bool init = false;
			if (!init) {
				static_logger.newfile(
					autofilename("log-", ".txt")
				);
				static_logger.set_on();
				init = true;
			}
			return static_logger;
		}

	/// 生成 "前缀 + 时间戳 + 后缀" 形式的自动文件名
	inline static std::string autofilename(std::string prefix, std::string postfix) {
			return prefix + _datetime_simple() + postfix;
		}

		void newfile_auto();

		inline void newfile(std::string name) {
			out = std::ofstream(name);
		}

		inline void set_on() { on = true; }
		inline void set_off() { on = false; }

		inline Logger& info(std::string str) {
			if (on)
				out << _datetime_simple()
				<< " [INFO] " << str << "\n";
			return *this;
		}

		inline Logger& error(std::string str) {
			if (on)
				out << _datetime_simple()
				<< " [ERROR] " << str << "\n";
			return *this;
		}

		inline Logger& operator<<(std::string str) {
			return info(str);
		}
		inline Logger& linesplit() {
			return info("----------------------------\n");
		}
		inline Logger& datetime() {
			return info(_datetime());
		}
	/// 压入一个新计时器
	inline void timer_start() {
		timers.push_back(timer());
	}
	/// 弹出栈顶计时器并返回经过时间（unit 换算，默认秒）
	inline double timer_end(double unit = sec) {
			if (timers.size() == 0) {
				return 0.0;
			}
			double ret = timers.back().get(unit);
			timers.pop_back();
			return ret;
		}
		inline void flush() {
			out.flush();
		}
	};

	/// 写一条 INFO 日志
	inline void log_info(std::string msg) {
		Logger::instance().info(msg);
	}

	/// 写一条 ERROR 日志
	inline void log_error(std::string msg) {
		Logger::instance().error(msg);
	}

	/**
	 * @brief 单个函数的性能档案：调用计数与累计耗时（栈式计时）。
	 */
	struct profile {
		size_t ncalls = 0;
		double time = 0;
		std::vector<timer> timers;
		size_t max_depth = 100;
		profile() {
			enter();
		}
		void enter() {
			if (timers.size() == max_depth)
				throw std::runtime_error("Exceed max depth.");
			timers.push_back(timer());
			ncalls++;
		}
		void exit() {
			if (timers.size() == 0)
				throw std::runtime_error("Why profiler has 0 timer?");
			time += timers.back().get(millisec);
			timers.pop_back();
		}
	};

	inline std::string truncate_name(const std::string &name, size_t max_nchar)
	{
		std::string ret;
		ret.reserve(max_nchar);
		ret.assign(name.begin(), name.begin() + max_nchar - 3);
		ret += "...";
		return ret;
	}

	/**
	 * @brief RAII 函数剖面器：构造计时进入、析构退出，按函数名聚合。
	 *
	 * 配合 FunctionProfiler 宏使用（在函数入口声明
	 * `volatile profiler _profilehelper_(__FUNCTION__);`），
	 * print_profiler 输出各函数的调用次数与累计耗时。
	 */
	struct profiler {
		static std::map<std::string, profile*> profiles;
		static bool on;
		std::string current_identifier;
		profile* current_profile = nullptr;

		profiler(const std::string& function_identifier) {
			if (!on) { return; }
			if (function_identifier.size() > 35) {
				current_identifier.assign(function_identifier.begin(), function_identifier.begin() + 25);
				current_identifier += "...";
			}
			else {
				current_identifier = function_identifier;
			}

			auto iter = profiles.find(current_identifier);
			if (iter == profiles.end()) {
				current_profile = new profile();
				profiles.insert({ current_identifier, current_profile });
			}
			else {
				current_profile = iter->second;
				current_profile->enter();
			}
		}

		~profiler() {
			if (!on) { return; }
			current_profile->exit();
		}

		inline static void init_profiler() {
			profiles.clear();
		}

		inline static void close_profiler() {
			profiler::on = false;
		}

		inline static void start_profiler() {
			profiler::on = true;
		}

		inline static void print_profiler() {
			fmt::print("{}\n", get_all_profiles_v2());
		}

		inline static double get_time(const std::string& profilename) {
			auto iter = profiles.find(profilename);
			if (iter == profiles.end()) return 0.0;
			else return iter->second->time;
		}

		inline static size_t get_ncalls(const std::string& profilename) {
			auto iter = profiles.find(profilename);
			if (iter == profiles.end()) return 0;
			else return iter->second->ncalls;
		}

		inline static std::string get_all_profiles() {
			if (profiles.empty()) {
				return "No profiles.";
			}
			std::string ret;
			for (const auto &profile : profiles) {
				ret += fmt::format("[{:^38s}] Calls = {:^3d} Time = {:^4f} ms\n",
					profile.first/*truncate_name(profile.first, 25)*/, profile.second->ncalls, profile.second->time);
			}
			return ret;
		}

		inline static std::vector<std::tuple<std::string, size_t, double>> get_profiles_info()
		{
			using profile_info_t = std::tuple<std::string, size_t, double>;
			std::vector<profile_info_t> ret;
			ret.reserve(profiles.size());

#if __cplusplus >= 201703L
			for (auto&& [name, profile] : profiles) {
				ret.emplace_back(name, profile->ncalls, profile->time);
			}
#else
			for (auto &name_profile : profiles) {
				auto& name = name_profile.first;
				auto& profile = name_profile.second;
				ret.emplace_back(name, profile->ncalls, profile->time);
			}
#endif

			std::sort(ret.begin(), ret.end(), [](const profile_info_t& lhs, const profile_info_t& rhs)
				{
					return std::get<2>(lhs) < std::get<2>(rhs);
				});

			return ret;
		}

		static std::string get_all_profiles_v2();
	};

/// 在函数入口声明 RAII 剖面器（配合 profiler::print_profiler 输出）
#define FunctionProfiler volatile profiler _profilehelper_(__FUNCTION__)

	extern Logger logger;

	/// 格式化后同时打印到 stdout 并写入日志
	template <typename... Ty>
	void print_and_log(std::string fmt_str, Ty&&...args) {
		std::string str = format(fmt_str, std::forward<Ty>(args)...);
		fmt::print(str);
		logger << str;
		logger.flush();
	}

	/**
	 * @brief 在线统计量：边记录边累计均值 / 方差 / 标准差。
	 *
	 * @tparam Ty 记录值类型（须可转 double）
	 */
	template<typename Ty>
	struct Statistic
	{
		/// 原始记录
		std::vector<Ty> records;
		/// 累计和
		double sum = 0;
		/// 累计平方和
		double sum_sqr = 0;
		/// 记录条数
		size_t shots = 0;

		inline Ty simple_record(Ty r) {
			double r_float = static_cast<double>(r);
			sum += r_float;
			sum_sqr += r_float * r_float;
			shots++;
			return r;
		}		

		inline Ty record(Ty r) {
			double r_float = static_cast<double>(r);
			sum += r_float;
			sum_sqr += r_float * r_float;
			shots++;
			return r;
		}

		inline double mean() const
		{
			return sum / shots;
		}

		inline double mean_sqr() const
		{
			return sum_sqr / shots;
		}

		inline double Var() const
		{
			return mean_sqr() - mean() * mean();
		}

		inline double std() const
		{
			return std::sqrt(Var());
		}
	};

	using StatisticDouble = Statistic<double>;  ///< double 统计别名
	using StatisticInt = Statistic<int>;        ///< int 统计别名
	using StatisticSize = Statistic<size_t>;    ///< size_t 统计别名

	/**
	 * @brief 实验结果输出器：按 Python 类模板生成 .py 结果文件。
	 *
	 * 以模板字符串填充实验名 / 变量 / 结果 / 时间 / 剖面信息，
	 * 输出可直接被 Python 侧导入分析的 Experiment-*.py 文件。
	 */
	struct Outputter
	{
		// std::string template_str;
		// std::string template_filename;
		std::string result_directory;
		std::string random_hash;

		inline static const std::string template_str =
			"class { EXPERIMENT_NAME }:\n"
			"    name = { NAME }\n"
			"    variables = { VARIABLES }\n"
			"    result = { RESULT }\n"
			"    datetime = { DATETIME }\n"
			"    profiler = { PROFILER }\n";
		inline static const std::string name_tag = "{ NAME }";
		inline static const std::string experiment_name_tag = "{ EXPERIMENT_NAME }";
		inline static const std::string variables_tag = "{ VARIABLES }";
		inline static const std::string result_tag = "{ RESULT }";
		inline static const std::string datetime_tag = "{ DATETIME }";
		inline static const std::string profiler_tag = "{ PROFILER }";

		/* Default constructor of class Outputter
		* Initializes the result directory to the current directory.
		*/
		Outputter()
			: result_directory(".")
		{
		}

		/* Constructor of class Outputter
		* Initializes the result directory to the specified directory.
		*/
		Outputter(std::string_view result_directory_)
			: result_directory(result_directory_)
		{
		}

		inline std::string set_output(std::string name, std::string variable_str, std::string result_str)
		{
			// init with template
			std::string output = template_str;
			size_t pos, length;

			random_hash = get_random_hash_str(variable_str);
			pos = output.find(experiment_name_tag);
			length = experiment_name_tag.size();
			output.replace(pos, length, fmt::format("Experiment_{}", random_hash));

			pos = output.find(name_tag);
			length = name_tag.size();
			output.replace(pos, length, fmt::format("\"{}\"", name));

			pos = output.find(variables_tag);
			length = variables_tag.size();
			output.replace(pos, length, variable_str);

			pos = output.find(result_tag);
			length = result_tag.size();
			output.replace(pos, length, result_str);

			pos = output.find(datetime_tag);
			length = datetime_tag.size();
			output.replace(pos, length, fmt::format("\"{}\"", _datetime_simple()));

			pos = output.find(profiler_tag);
			length = profiler_tag.size();
			output.replace(pos, length, fmt::format("{}", profiler::get_profiles_info()));

			return output;
		}

		inline void make_output(std::string name, std::string variable_str, std::string result_str)
		{
			std::string output = set_output(name, variable_str, result_str);

			std::string result_file = result_directory + "/Experiment-" + _datetime_simple() + "-" + random_hash + ".py";

			FILE* fp = fopen(result_file.c_str(), "w+");
			if (!fp) {
				fmt::print("Cannot open such file.\n");
				fmt::print("result_file = {}\n", result_file);
				throw_general_runtime_error();
			}

			fwrite(output.c_str(), sizeof(output[0]), output.size(), fp);

			fclose(fp);
		}

		template<typename ArgumentType, typename ResultTy>
		void make_output(const ArgumentType& arguments, const ResultTy& result)
		{
			std::string name = ArgumentType::name;
			if (!arguments.experimentname.empty())
			{
				name += "_";
				name += arguments.experimentname;
			}
			make_output(name,
				fmt::format("{}", arguments.to_mapstring()), fmt::format("{}", result));
		}
	};
} // namespace qram_simulator