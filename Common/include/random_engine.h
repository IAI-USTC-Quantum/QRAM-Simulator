#pragma once

#include <random>
#include <ctime>

namespace qram_simulator {
	/// Random seed type (same type as time_t)
	using seed_t = decltype(std::time(0));
	/// Random engine type: MT19937-64
	using engine_t = std::mt19937_64;

	/**
	 * @brief Global random engine singleton (MT19937-64).
	 *
	 * The whole library shares one random source: QRAM memory generation,
	 * input sampling, and output sampling are all driven by this engine;
	 * after set_seed the entire simulation pipeline is reproducible
	 * (default seed 10101).
	 */
	struct random_engine
	{
		/// Current seed
		seed_t seed = 10101;
		/// Engine instance
		std::mt19937_64 reng;

		random_engine() {}

		/// Get the process-wide unique instance
		inline static random_engine& get_instance()
		{
			static random_engine inst;
			return inst;
		}

		/// Reference to the engine (instance method)
		inline engine_t& _get_engine()
		{
			return reng;
		}

		/// Const reference to the engine (instance method)
		inline const engine_t& _get_engine() const
		{
			return reng;
		}

		/// Reference to the global engine
		inline static engine_t& get_engine()
		{
			return get_instance()._get_engine();
		}

		/// Generate one U(0,1) random number (instance method)
		inline double _rng()
		{
			static std::uniform_real_distribution<double> ud(0, 1);
			return ud(reng);
		}

		/// Generate one U(0,1) random number
		inline static double rng()
		{
			return get_instance()._rng();
		}

		/// Generate one U(0,1) random number (alias of _rng)
		inline double _uniform01()
		{
			return _rng();
		}

		/// Generate one U(0,1) random number (alias of rng)
		inline static double uniform01()
		{
			return get_instance()._uniform01();
		}

		/// Generate one U(a,b) random number (instance method)
		inline double _uniform(double a, double b)
		{
			return a + (b - a) * _rng();
		}

		/// Generate one U(a,b) random number
		inline static double uniform(double a, double b)
		{
			return get_instance()._uniform(a, b);
		}

		/// Generate one random integer in [a,b] (instance method)
		inline int _randint(int a, int b)
		{
			return std::uniform_int_distribution<int>(a, b)(get_engine());
		}

		/// Generate one random integer in [a,b]
		inline static int randint(int a, int b)
		{
			return get_instance()._randint(a, b);
		}

		/// Set the seed (instance method)
		inline void _set_seed(seed_t _seed)
		{
			seed = _seed;
			reng.seed(_seed);
		}

		/// Set the seed of the global random engine
		inline static void set_seed(seed_t _seed)
		{
			get_instance()._set_seed(_seed);
		}

		/// Reset the seed from the current random number and return the new seed
		inline seed_t reseed()
		{
			_set_seed(seed_t(_rng() * (std::numeric_limits<seed_t>::max)()));
			return seed;
		}
		/// Return the current seed (instance method)
		inline seed_t _get_seed() { return seed; }

		/// Return the current seed of the global random engine
		inline static seed_t get_seed()
		{
			return get_instance()._get_seed();
		}

		/// Set the seed from the current system time and return that seed
		inline static seed_t time_seed()
		{
			seed_t seed = time(0);
			get_instance()._set_seed(seed);
			return seed;
		}
	};

	/**
	 * @brief Generate a 16-digit hexadecimal hash string by combining value with a random number.
	 * @param value Value participating in the hash
	 * @return 16-digit hexadecimal string
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

	/// Generate a purely random 16-digit hexadecimal hash string
	inline std::string get_random_hash_str()
	{
		size_t hashed = std::hash<double>()(random_engine::get_instance().uniform01());

		char buf[64];
		std::sprintf(buf, "%.16zX", hashed);
		return buf;
	}

}// namespace qram_simulator
