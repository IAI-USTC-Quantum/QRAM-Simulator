#pragma once

#include <random>
#include <ctime>

namespace qram_simulator {
	/// 随机种子类型（与 time_t 同型）
	using seed_t = decltype(std::time(0));
	/// 随机引擎类型：MT19937-64
	using engine_t = std::mt19937_64;

	/**
	 * @brief 全局随机引擎单例（MT19937-64）。
	 *
	 * 全库共享同一随机源：QRAM 的内存生成、输入采样与输出采样均由
	 * 该引擎驱动，set_seed 后即可复现整条仿真链路（默认种子 10101）。
	 */
	struct random_engine
	{
		/// 当前种子
		seed_t seed = 10101;
		/// 引擎实例
		std::mt19937_64 reng;

		random_engine() {}

		/// 取进程级唯一实例
		inline static random_engine& get_instance()
		{
			static random_engine inst;
			return inst;
		}

		/// 引擎引用（实例方法）
		inline engine_t& _get_engine()
		{
			return reng;
		}

		/// 引擎 const 引用（实例方法）
		inline const engine_t& _get_engine() const
		{
			return reng;
		}

		/// 全局引擎引用
		inline static engine_t& get_engine()
		{
			return get_instance()._get_engine();
		}

		/// 产生一个 U(0,1) 随机数（实例方法）
		inline double _rng()
		{
			static std::uniform_real_distribution<double> ud(0, 1);
			return ud(reng);
		}

		/// 产生一个 U(0,1) 随机数
		inline static double rng()
		{
			return get_instance()._rng();
		}

		/// 产生一个 U(0,1) 随机数（_rng 别名）
		inline double _uniform01()
		{
			return _rng();
		}

		/// 产生一个 U(0,1) 随机数（rng 别名）
		inline static double uniform01()
		{
			return get_instance()._uniform01();
		}

		/// 产生一个 U(a,b) 随机数（实例方法）
		inline double _uniform(double a, double b)
		{
			return a + (b - a) * _rng();
		}

		/// 产生一个 U(a,b) 随机数
		inline static double uniform(double a, double b)
		{
			return get_instance()._uniform(a, b);
		}

		/// 产生一个 [a,b] 内的随机整数（实例方法）
		inline int _randint(int a, int b)
		{
			return std::uniform_int_distribution<int>(a, b)(get_engine());
		}

		/// 产生一个 [a,b] 内的随机整数
		inline static int randint(int a, int b)
		{
			return get_instance()._randint(a, b);
		}

		/// 设置种子（实例方法）
		inline void _set_seed(seed_t _seed)
		{
			seed = _seed;
			reng.seed(_seed);
		}

		/// 设置全局随机引擎种子
		inline static void set_seed(seed_t _seed)
		{
			get_instance()._set_seed(_seed);
		}

		/// 以当前随机数重设种子并返回新种子
		inline seed_t reseed()
		{
			_set_seed(seed_t(_rng() * (std::numeric_limits<seed_t>::max)()));
			return seed;
		}
		/// 返回当前种子（实例方法）
		inline seed_t _get_seed() { return seed; }

		/// 返回全局随机引擎的当前种子
		inline static seed_t get_seed()
		{
			return get_instance()._get_seed();
		}

		/// 以当前系统时间设置种子并返回该种子
		inline static seed_t time_seed()
		{
			seed_t seed = time(0);
			get_instance()._set_seed(seed);
			return seed;
		}
	};

	/**
	 * @brief 由 value 与一个随机数拼合生成 16 位十六进制哈希串。
	 * @param value 参与哈希的值
	 * @return 16 位十六进制字符串
	 */
	template<typename Ty>
	std::string get_random_hash_str(const Ty& value)
	{
		size_t hashed = std::hash<Ty>()(value) ^
			std::hash<double>()(random_engine::get_instance().uniform01());

		char buf[64];
		std::sprintf(buf, "%.16zX", hashed);
		return buf;
	}

	/// 生成纯随机的 16 位十六进制哈希串
	inline std::string get_random_hash_str()
	{
		size_t hashed = std::hash<double>()(random_engine::get_instance().uniform01());

		char buf[64];
		std::sprintf(buf, "%.16zX", hashed);
		return buf;
	}

}// namespace qram_simulator
