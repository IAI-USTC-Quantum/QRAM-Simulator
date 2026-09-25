#pragma once
#include <vector>
#include <cstddef>

namespace qram_simulator {

	/**
	 * @brief Python 风格的整数 range 迭代器。
	 *
	 * 支持 range(stop) 与 range(start, stop[, step])，可用于基于范围的
	 * for 循环：`for (size_t i : range(4))`。
	 */
	class range {
	private:
		size_t start, stop, step;

	public:
		/// 构造 [0, stop) 区间
		range(size_t stop) : start(0), stop(stop), step(1) {}
		/// 构造 [start, stop) 区间，步长 step
		range(size_t start, size_t stop, size_t step = 1)
            : start(start), stop(stop), step(step) {}

		/// 迭代器：按步长推进，以 stop 为界
		class iterator {
		private:
			size_t current, step, stop;

		public:
			iterator() : current(0), step(0), stop(0) {}
			iterator(size_t current, size_t step, size_t stop)
				: current(current), step(step), stop(stop) {}

			/// 解引用：返回当前位置
			inline size_t operator*() const { return current; }

			/// 前进一步
			inline iterator& operator++() {
				current += step;
				return *this;
			}

			/// 判断是否仍在区间内（与哨兵 end 比较）
			inline bool operator!=(const iterator& other) const {
				return (step > 0) ? (current < stop) : (current > stop);
			}
		};

		/// 起始迭代器
		inline iterator begin() const { return iterator(start, step, stop); }
		/// 终止哨兵迭代器
		inline iterator end() const { return iterator(stop, step, stop); }
	};

	/**
	 * @brief 任意多个容器的笛卡尔积迭代器。
	 *
	 * 类似 Python 的 itertools.product：按最后一个容器优先的顺序
	 * 遍历所有组合，每次解引用得到各容器当前元素的 tuple。
	 * 用法：`for (auto [i, j] : product(vecA, vecB))`。
	 */
	template <typename... Containers>
	class product {
	private:
		std::tuple<Containers...> containers;

		/// 取第 I 个容器的起始迭代器
		template <size_t I>
		auto get_begin() const {
			return std::begin(std::get<I>(containers));
		}

		/// 取第 I 个容器的终止迭代器
		template <size_t I>
		auto get_end() const {
			return std::end(std::get<I>(containers));
		}

		/// 进位式递增：从第 I 个容器起逐个推进，全满则返回 false
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

		/// 打包全部容器的起始迭代器
		template <size_t... Is>
		auto get_all_begins(std::index_sequence<Is...>) const {
			return std::make_tuple(get_begin<Is>()...);
		}

	public:
		/// 以若干容器构造笛卡尔积视图
		product(const Containers&... cs)
			: containers(cs...) {}

		//template<typename... Args>
		//product(Args&&... cs)
		//    : containers(std::forward<Args>(cs)...) {}

		/// 笛卡尔积迭代器
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

			/// 解引用：返回各容器当前元素组成的 tuple
			auto operator*() const {
				return std::apply([](auto&&... its) {
					return std::make_tuple(*its...);
					}, current);
			}

			/// 前进到下一个组合
			iterator& operator++() {
				is_end = !parent->increment<sizeof...(Containers) - 1>(current);
				return *this;
			}

			/// 与哨兵 end 比较（是否遍历完毕）
			bool operator!=(const iterator& other) const {
				return is_end != other.is_end;
			}
		};

		/// 起始迭代器
		iterator begin() const { return iterator(this); }
		/// 终止哨兵迭代器
		iterator end() const { return iterator(this, true); }
	};
}
