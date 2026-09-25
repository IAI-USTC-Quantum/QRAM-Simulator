#pragma once
#include <vector>
#include <cstddef>

namespace qram_simulator {

	/**
	 * @brief Python-style integer range iterator.
	 *
	 * Supports range(stop) and range(start, stop[, step]); usable in
	 * range-based for loops: `for (size_t i : range(4))`.
	 */
	class range {
	private:
		size_t start, stop, step;

	public:
		/// Construct the interval [0, stop)
		range(size_t stop) : start(0), stop(stop), step(1) {}
		/// Construct the interval [start, stop) with step size step
		range(size_t start, size_t stop, size_t step = 1)
            : start(start), stop(stop), step(step) {}

		/// Iterator: advances by the step size, bounded by stop
		class iterator {
		private:
			size_t current, step, stop;

		public:
			iterator() : current(0), step(0), stop(0) {}
			iterator(size_t current, size_t step, size_t stop)
				: current(current), step(step), stop(stop) {}

			/// Dereference: return the current position
			inline size_t operator*() const { return current; }

			/// Advance one step
			inline iterator& operator++() {
				current += step;
				return *this;
			}

			/// Check whether still inside the interval (compared against the end sentinel)
			inline bool operator!=(const iterator& other) const {
				return (step > 0) ? (current < stop) : (current > stop);
			}
		};

		/// Iterator to the beginning
		inline iterator begin() const { return iterator(start, step, stop); }
		/// End sentinel iterator
		inline iterator end() const { return iterator(stop, step, stop); }
	};

	/**
	 * @brief Cartesian product iterator over any number of containers.
	 *
	 * Similar to Python's itertools.product: traverses all combinations
	 * with the last container varying fastest; each dereference yields a
	 * tuple of the current elements of all containers.
	 * Usage: `for (auto [i, j] : product(vecA, vecB))`.
	 */
	template <typename... Containers>
	class product {
	private:
		std::tuple<Containers...> containers;

		/// Get the begin iterator of the I-th container
		template <size_t I>
		auto get_begin() const {
			return std::begin(std::get<I>(containers));
		}

		/// Get the end iterator of the I-th container
		template <size_t I>
		auto get_end() const {
			return std::end(std::get<I>(containers));
		}

		/// Carry-style increment: advance the containers one by one starting from the I-th; return false when all are exhausted
		template <size_t I>
		bool increment(std::tuple<typename Containers::iterator...>& current) const {
			auto& it = std::get<I>(current);
			++it;
			if (it != get_end<I>()) return true;
			if constexpr (I > 0) {
				it = get_begin<I>();
				return increment<I - 1>(current);
			}
			return false;
		}

		/// Pack the begin iterators of all containers
		template <size_t... Is>
		auto get_all_begins(std::index_sequence<Is...>) const {
			return std::make_tuple(get_begin<Is>()...);
		}

	public:
		/// Construct a Cartesian product view from several containers
		product(const Containers&... cs)
			: containers(cs...) {}

		//template<typename... Args>
		//product(Args&&... cs)
		//    : containers(std::forward<Args>(cs)...) {}

		/// Cartesian product iterator
		class iterator {
		private:
			const product* parent;
			std::tuple<typename Containers::iterator...> current;
			bool is_end;

		public:
			iterator() : parent(nullptr), is_end(true) {}

			iterator(const product* parent, bool is_end = false)
				: parent(parent), is_end(is_end) {
				if (!is_end) {
					constexpr auto size = sizeof...(Containers);
					current = parent->get_all_begins(std::make_index_sequence<size>{});
				}
			}

			/// Dereference: return a tuple of the current elements of all containers
			auto operator*() const {
				return std::apply([](auto&&... its) {
					return std::make_tuple(*its...);
					}, current);
			}

			/// Advance to the next combination
			iterator& operator++() {
				is_end = !parent->increment<sizeof...(Containers) - 1>(current);
				return *this;
			}

			/// Compare against the end sentinel (whether traversal is finished)
			bool operator!=(const iterator& other) const {
				return is_end != other.is_end;
			}
		};

		/// Iterator to the beginning
		iterator begin() const { return iterator(this); }
		/// End sentinel iterator
		iterator end() const { return iterator(this, true); }
	};
}
