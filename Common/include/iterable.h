#pragma once
#include <vector>
#include <cstddef>

namespace qram_simulator {

    class range {
    private:
        size_t start, stop, step;

    public:
        range(size_t stop) : start(0), stop(stop), step(1) {}
        range(size_t start, size_t stop, size_t step = 1)
            : start(start), stop(stop), step(step) {}

        class iterator {
        private:
            size_t current, step, stop;

        public:
            iterator() : current(0), step(0), stop(0) {}
            iterator(size_t current, size_t step, size_t stop)
                : current(current), step(step), stop(stop) {}

            inline size_t operator*() const { return current; }

            inline iterator& operator++() {
                current += step;
                return *this;
            }

            inline bool operator!=(const iterator& other) const {
                return (step > 0) ? (current < stop) : (current > stop);
            }
        };

        inline iterator begin() const { return iterator(start, step, stop); }
        inline iterator end() const { return iterator(stop, step, stop); }
    };

    template <typename... Containers>
    class product {
    private:
        std::tuple<Containers...> containers;

        template <size_t I>
        auto get_begin() const {
            return std::begin(std::get<I>(containers));
        }

        template <size_t I>
        auto get_end() const {
            return std::end(std::get<I>(containers));
        }

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

        template <size_t... Is>
        auto get_all_begins(std::index_sequence<Is...>) const {
            return std::make_tuple(get_begin<Is>()...);
        }

    public:
        product(const Containers&... cs)
            : containers(cs...) {}

        //template<typename... Args>
        //product(Args&&... cs)
        //    : containers(std::forward<Args>(cs)...) {}

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

            auto operator*() const {
                return std::apply([](auto&&... its) {
                    return std::make_tuple(*its...);
                    }, current);
            }

            iterator& operator++() {
                is_end = !parent->increment<sizeof...(Containers) - 1>(current);
                return *this;
            }

            bool operator!=(const iterator& other) const {
                return is_end != other.is_end;
            }
        };

        iterator begin() const { return iterator(this); }
        iterator end() const { return iterator(this, true); }
    };
}